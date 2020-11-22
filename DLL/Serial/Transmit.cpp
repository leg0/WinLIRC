#include <windows.h>

#include <winlirc/PluginApi.h>

#include "Transmit.h"

HANDLE	tPort = nullptr;
INT64	freq;		//the frequency used by PerformanceCounter
INT64	lasttime;	//last time counter was queried
unsigned long pulse_width = 13; /* pulse/space ratio of 50/50 */
unsigned long space_width = 13; /* 1000000/freq-pulse_width */
int pulse_byte_length=78; //usecs per byte (tx soft carrier)
unsigned transmittertype;
static sbuf send_buffer;
extern winlirc_interface winlirc;

#define MAXPULSEBYTES 256
int pulsedata[MAXPULSEBYTES];

void	(*on)			(void);				//pointer to on function for hard carrier transmitters
void	(*off)			(void);				//pointer to off function for hard carrier transmitters

int		(*send_pulse)	(unsigned long);	//pointer to send_pulse function
int		(*send_space)	(unsigned long);	//pointer to send_space function

void on_dtr(void)
{
    EscapeCommFunction(tPort,SETDTR);
}

void off_dtr(void)
{
    EscapeCommFunction(tPort,CLRDTR);
}

void on_tx(void)
{
	SetCommBreak(tPort);
}
 
void off_tx_hard(void)
{
	ClearCommBreak(tPort);
}

void off_tx_soft(void)
{
}

int init_timer()
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&freq))
    return (-1);  // error hardware doesn't support performance counter
    QueryPerformanceCounter((LARGE_INTEGER*)&lasttime);
    return(0);
}

int uwait(unsigned long usecs)
// waits for a specified number of microseconds then returns 0.  returns -1 if timing is not supported
{
    __int64 end;
    end= lasttime + usecs * freq / 1000000;               // Convert microseconds to performance counter units per move.
    do
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&lasttime);
    } while (lasttime < end);
    lasttime=end;
    return (0);
}

int send_pulse_dtr_soft (unsigned long usecs)
{
	__int64 end;
	end= lasttime + usecs * freq / 1000000;
	do
	{
		on();
		uwait(pulse_width);
		off();
		uwait(space_width);
	} while (lasttime < end);
	return(1);
}

int send_space_hard_or_dtr(unsigned long length)
{
	if(length==0) return(1);
	off();
	uwait(length);
	return(1);
}

int send_pulse_hard (unsigned long length)
{
	if(length==0) return(1);
	on();
	uwait(length);
	return(1);
}

int send_pulse_tx_soft (unsigned long usecs)
{
	lasttime+=usecs * freq / 1000000;
	OVERLAPPED osWrite = {0};
	DWORD dwWritten;
	BOOL fRes;
	DWORD dwToWrite=usecs/pulse_byte_length;

	// Create this writes OVERLAPPED structure hEvent.
	osWrite.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (osWrite.hEvent == nullptr)
		// Error creating overlapped event handle.
		return FALSE;

	// Issue write.
	if (!WriteFile(tPort, pulsedata, dwToWrite, &dwWritten, &osWrite)) {
		if (GetLastError() != ERROR_IO_PENDING) { 
			// WriteFile failed, but it isn't delayed. Report error and abort.
			fRes = FALSE;
		}
		else {
			// Write is pending.
			if (!GetOverlappedResult(tPort, &osWrite, &dwWritten, TRUE))
				fRes = FALSE;
			else
				// Write operation completed successfully.
				fRes = TRUE;
		}
	}
	else
		// WriteFile completed immediately.
		fRes = TRUE;
	CloseHandle(osWrite.hEvent);
	return(fRes);
}

int send_space_tx_soft(unsigned long length)
{
	if(length==0) return(1);
	uwait(length);
	return(1);
}

void SetTransmitPort(HANDLE hCom,unsigned type)  // sets the serial port to transmit on
{
	tPort=hCom;
	void (*tmp) (void);
	switch (type%4)
	{
		case HARDCARRIER|TXTRANSMITTER:	//tx hard carrier
			on=on_tx;
			off=off_tx_hard;
			send_pulse=send_pulse_hard;
			send_space=send_space_hard_or_dtr;
			break;
		case HARDCARRIER:				//dtr hard carrier
			on=on_dtr;
			off=off_dtr;
			send_pulse=send_pulse_hard;
			send_space=send_space_hard_or_dtr;
			break;
		case TXTRANSMITTER:	//tx soft carrier
			on=off_tx_soft;
			off=off_tx_soft;
			send_pulse=send_pulse_tx_soft;
			send_space=send_space_tx_soft;
			break;
		default:						//dtr soft carrier
			on=on_dtr;
			off=off_dtr;
			send_pulse=send_pulse_dtr_soft;
			send_space=send_space_hard_or_dtr;
	}
	if (type&INVERTED) {
		tmp=on;
		on=off;
		off=tmp;
	}
	off();
	transmittertype=type;
	return;
}

bool config_transmitter(ir_remote *rem) //configures the transmitter for the specified remote
{
	if (winlirc.get_freq(rem)==0) winlirc.set_freq(rem,38000);				//default this should really be elsewhere
	if (winlirc.get_duty_cycle(rem)==0) winlirc.set_duty_cycle(rem,50);		//default this should really be elsewhere
	pulse_width=(unsigned long) winlirc.get_duty_cycle(rem)*10000/ winlirc.get_freq(rem);
	space_width=(unsigned long) 1000000L/ winlirc.get_freq(rem)-pulse_width;

	if (transmittertype==TXTRANSMITTER) //tx software carrier
	{
		DCB dcb;
		if(!GetCommState(tPort,&dcb))
		{
			CloseHandle(tPort);
			tPort=nullptr;
			return false;
		}
		dcb.BaudRate=CBR_115200;
		dcb.Parity=NOPARITY;
		dcb.StopBits=ONESTOPBIT;
		if (winlirc.get_freq(rem)<48000)
		{
			dcb.ByteSize=7;
			pulse_byte_length=78; // (1+bytesize+parity+stopbits+)/Baudrate*1E6
			if (winlirc.get_duty_cycle(rem)<50) for (int i=0;i<MAXPULSEBYTES;i++) pulsedata[i]=0x5b;
			else for (int i=0;i<MAXPULSEBYTES;i++) pulsedata[i]=0x12;
		} else {		
			dcb.ByteSize=8;
			pulse_byte_length=87; // (1+bytesize+parity+stopbits+)/Baudrate*1E6
			for (int i=0;i<MAXPULSEBYTES;i++) pulsedata[i]=0x55;
		}
		if(!SetCommState(tPort,&dcb))
		{
			CloseHandle(tPort);
			tPort=nullptr;
			//DEBUG("SetCommState failed.\n");
			return false;
		}
	}
	return true;
}

int Transmit(ir_ncode *data,struct ir_remote *rem, int repeats)
{
	//=======================
	DWORD	mypriorityclass;
	DWORD	mythreadpriority;
	HANDLE	myprocess;
	HANDLE	mythread;
	int		success;
	//=======================

    if (!rem || !data) return FALSE;

	success = FALSE;

	myprocess		= GetCurrentProcess();  //save these handles, because we'll need them several times
	mythread		= GetCurrentThread();
	mythreadpriority= GetPriorityClass(myprocess);  //store priority settings
    mypriorityclass	= GetThreadPriority(mythread);

	config_transmitter	(rem);
    SetPriorityClass	(myprocess,REALTIME_PRIORITY_CLASS);		//boost priority
    SetThreadPriority	(mythread,THREAD_PRIORITY_TIME_CRITICAL);

	init_timer			();

	if (winlirc.init_send(&send_buffer, rem, data, repeats)) {

		auto const length	= winlirc.get_send_buffer_length(&send_buffer);
		auto const signals	= winlirc.get_send_buffer_data(&send_buffer);

		for(int i=0; i<length; i++) {
			if(i%2==0)	send_pulse(signals[i]);
			else		send_space(signals[i]);
		}

		success = TRUE;
	}

    SetPriorityClass	(myprocess,mypriorityclass); //restore original priorities
    SetThreadPriority	(mythread,mythreadpriority);

	return success;
}

