/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2010 Ian Curtis
 */

#include "stdafx.h"
#include "LIRCDefines.h"
#include "Send.h"
#include "Globals.h"

#define LIRCD_EXACT_GAP_THRESHOLD 10000

HANDLE	tPort = NULL;
INT64	freq;		//the frequency used by PerformanceCounter
INT64	lasttime;	//last time counter was queried
unsigned long pulse_width = 13; /* pulse/space ratio of 50/50 */
unsigned long space_width = 13; /* 1000000/freq-pulse_width */
int pulse_byte_length=78; //usecs per byte (tx soft carrier)
unsigned transmittertype;

int sendBufferSum = 0;

#define MAXPULSEBYTES 256
int pulsedata[MAXPULSEBYTES];

void	(*on)			(void);				//pointer to on function for hard carrier transmitters
void	(*off)			(void);				//pointer to off function for hard carrier transmitters

int		(*send_pulse)	(unsigned long);	//pointer to send_pulse function
int		(*send_space)	(unsigned long);	//pointer to send_space function


void set_bit(ir_code *code,int bit,int data)
{
	(*code)&=~((((ir_code) 1)<<bit));
	(*code)|=((ir_code) (data ? 1:0)<<bit);
}

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

void off_tx_soft(void){;};

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
	sendBufferSum += usecs;
	//printf("Pulse %i\n",usecs);
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
	sendBufferSum += length;
	//printf("Space %i\n",length);
	if(length==0) return(1);
	off();
	uwait(length);
	return(1);
}

int send_pulse_hard (unsigned long length)
{
	sendBufferSum += length;
	if(length==0) return(1);
	on();
	uwait(length);
	return(1);
}

int send_pulse_tx_soft (unsigned long usecs)
{
	sendBufferSum += usecs;
	lasttime+=usecs * freq / 1000000;
	OVERLAPPED osWrite = {0};
	DWORD dwWritten;
	BOOL fRes;
	DWORD dwToWrite=usecs/pulse_byte_length;

	// Create this writes OVERLAPPED structure hEvent.
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
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

bool config_transmitter(struct ir_remote *rem) //configures the transmitter for the specified remote
{
	if (rem->freq==0) rem->freq=38000;				//default this should really be elsewhere
	if (rem->duty_cycle==0) rem->duty_cycle=50;		//default this should really be elsewhere
	pulse_width=(unsigned long) rem->duty_cycle*10000/rem->freq;
	space_width=(unsigned long) 1000000L/rem->freq-pulse_width;

	if (transmittertype==TXTRANSMITTER) //tx software carrier
	{
		DCB dcb;
		if(!GetCommState(tPort,&dcb))
		{
			CloseHandle(tPort);
			tPort=NULL;
			return false;
		}
		dcb.BaudRate=CBR_115200;
		dcb.Parity=NOPARITY;
		dcb.StopBits=ONESTOPBIT;
		if (rem->freq<48000)
		{
			dcb.ByteSize=7;
			pulse_byte_length=78; // (1+bytesize+parity+stopbits+)/Baudrate*1E6
			if (rem->duty_cycle<50) for (int i=0;i<MAXPULSEBYTES;i++) pulsedata[i]=0x5b;
			else for (int i=0;i<MAXPULSEBYTES;i++) pulsedata[i]=0x12;
		} else {		
			dcb.ByteSize=8;
			pulse_byte_length=87; // (1+bytesize+parity+stopbits+)/Baudrate*1E6
			for (int i=0;i<MAXPULSEBYTES;i++) pulsedata[i]=0x55;
		}
		if(!SetCommState(tPort,&dcb))
		{
			CloseHandle(tPort);
			tPort=NULL;
			//DEBUG("SetCommState failed.\n");
			return false;
		}
	}
	return true;
}

//===========================================================================


/*
  sending stuff
*/

inline void send_header(struct ir_remote *remote)
{
	if(has_header(remote))
	{
		send_pulse(remote->phead);
		send_space(remote->shead);
	}
}

inline void send_foot(struct ir_remote *remote)
{
	if(has_foot(remote))
	{
		send_space(remote->sfoot);
		send_pulse(remote->pfoot);
	}
}

inline void send_lead(struct ir_remote *remote)
{
	if(remote->plead!=0)
	{
		send_pulse(remote->plead);
	}
}

inline void send_trail(struct ir_remote *remote)
{
	if(remote->ptrail!=0)
	{
		send_pulse(remote->ptrail);
	}
}

inline void send_data(struct ir_remote *remote,ir_code data,int bits,int done)
{
	int i;
	int all_bits = bit_count(remote);
	int toggle_bit_mask_bits = bits_set(remote->toggle_bit_mask);
	ir_code mask;
	
	data=reverse(data,bits);
	if(is_rcmm(remote))
	{
		mask=(ir_code)1<<(all_bits-1-done);
		if(bits%2 || done%2)
		{
			return;
		}
		for(i=0;i<bits;i+=2,mask>>=2)
		{
			switch(data&3)
			{
			case 0:
				send_pulse(remote->pzero);
				send_space(remote->szero);
				break;
			/* 2 and 1 swapped due to reverse() */
			case 2:
				send_pulse(remote->pone);
				send_space(remote->sone);
				break;
			case 1:
				send_pulse(remote->ptwo);
				send_space(remote->stwo);
				break;
			case 3:
				send_pulse(remote->pthree);
				send_space(remote->sthree);
				break;
			}
			data=data>>2;
		}
		return;
	}
	else if(is_xmp(remote))
	{
		if(bits%4 || done%4)
		{
			return;
		}
		for(i = 0; i < bits; i += 4)
		{
			ir_code nibble;

			nibble = reverse(data & 0xf, 4);
			send_pulse(remote->pzero);
			send_space((unsigned long)(remote->szero + nibble*remote->sone));
			data >>= 4;
		}
		return;
	}

	mask=((ir_code) 1)<<(all_bits-1-done);
	for(i=0;i<bits;i++,mask>>=1)
	{
		if(has_toggle_bit_mask(remote) && mask&remote->toggle_bit_mask)
		{
			if(toggle_bit_mask_bits == 1)
			{
				/* backwards compatibility */
				data &= ~((ir_code) 1);
				if(remote->toggle_bit_mask_state&mask)
				{
					data |= (ir_code) 1;
				}
			}
			else
			{
				if(remote->toggle_bit_mask_state&mask)
				{
					data ^= (ir_code) 1;
				}
			}
		}
		if(has_toggle_mask(remote) &&
		   mask&remote->toggle_mask &&
		   remote->toggle_mask_state%2)
		{
			data ^= 1;
		}
		if(data&1)
		{
			if(is_biphase(remote))
			{
				
				if(mask&remote->rc6_mask)
				{
					send_space(2*remote->sone);
					send_pulse(2*remote->pone);
				}
				else
				{
					send_space(remote->sone);
					send_pulse(remote->pone);
				}
			}
			else if(is_space_first(remote))
			{
				send_space(remote->sone);
				send_pulse(remote->pone);
			}
			else
			{
				send_pulse(remote->pone);
				send_space(remote->sone);
			}
		}
		else
		{
			if(mask&remote->rc6_mask)
			{
				send_pulse(2*remote->pzero);
				send_space(2*remote->szero);
			}
			else if(is_space_first(remote))
			{
				send_space(remote->szero);
				send_pulse(remote->pzero);
			}
			else
			{
				send_pulse(remote->pzero);
				send_space(remote->szero);
			}
		}
		data=data>>1;
	}
}

inline void send_pre(struct ir_remote *remote)
{
	if(has_pre(remote))
	{
		send_data(remote,remote->pre_data,remote->pre_data_bits,0);
		if(remote->pre_p>0 && remote->pre_s>0)
		{
			send_pulse(remote->pre_p);
			send_space(remote->pre_s);
		}
	}
}

inline void send_post(struct ir_remote *remote)
{
	if(has_post(remote))
	{
		if(remote->post_p>0 && remote->post_s>0)
		{
			send_pulse(remote->post_p);
			send_space(remote->post_s);
		}
		send_data(remote,remote->post_data,remote->post_data_bits,
			  remote->pre_data_bits+remote->bits);
	}
}

inline void send_repeat(struct ir_remote *remote)
{
	send_lead(remote);
	send_pulse(remote->prepeat);
	send_space(remote->srepeat);
	send_trail(remote);
}

inline void send_code(struct ir_remote *remote,ir_code code, int repeat)
{
	if(!repeat || !(remote->flags&NO_HEAD_REP))
		send_header(remote);
	send_lead(remote);
	send_pre(remote);
	send_data(remote,code,remote->bits,remote->pre_data_bits);
	send_post(remote);
	send_trail(remote);
	if(!repeat || !(remote->flags&NO_FOOT_REP))
		send_foot(remote);

	if(!repeat && remote->flags&NO_HEAD_REP && remote->flags&CONST_LENGTH)
	{
		sendBufferSum -= remote->phead+remote->shead;
	}
}

static void send_signals(lirc_t *signals, int n)
{
	int i;
	
	for(i=0; i<n; i++)
	{
		if (i%2)	send_space(signals[i]);
        else		send_pulse(signals[i]);
	}

	off();
}

int init_send(struct ir_remote *remote,struct ir_ncode *code, int repeats)
{
	int repeat = 0;
	int i = 0;
	
	if(is_grundig(remote) || is_goldstar(remote) || is_serial(remote) || is_bo(remote))
	{
		return(0);
	}

	sendBufferSum = 0;

	remote->repeat_countdown = max(remote->min_repeat,repeats);
	
 init_send_loop:
	if(repeat && has_repeat(remote))
	{
		if(remote->flags&REPEAT_HEADER && has_header(remote))
		{
			send_header(remote);
		}
		send_repeat(remote);
	}
	else
	{
		if(!is_raw(remote))
		{
			send_code(remote, code->code, repeat);

			if(has_toggle_mask(remote))
			{
				remote->toggle_mask_state++;

				if(remote->toggle_mask_state==4)
				{
					remote->toggle_mask_state=2;
				}
			}
		}
		else
		{
			if(code->signals==NULL)
			{
				return 0;
			}

			send_signals(code->signals, code->length);

			for(i=0; i<code->length; i++)
			{
				sendBufferSum+=code->signals[i];
			}
		}
	}

	if(has_repeat_gap(remote) && repeat && has_repeat(remote))
	{
		remote->min_remaining_gap=remote->repeat_gap;
		remote->max_remaining_gap=remote->repeat_gap;
	}
	else if(is_const(remote))
	{
		if(min_gap(remote)>sendBufferSum)
		{
			remote->min_remaining_gap=min_gap(remote)-sendBufferSum;
			remote->max_remaining_gap=max_gap(remote)-sendBufferSum;
		}
		else
		{
			remote->min_remaining_gap=min_gap(remote);
			remote->max_remaining_gap=max_gap(remote);
			return(0);
		}
	}
	else
	{
		remote->min_remaining_gap=min_gap(remote);
		remote->max_remaining_gap=max_gap(remote);
	}

	send_space(remote->min_remaining_gap);

	if(remote->repeat_countdown>0)
	{
		remote->repeat_countdown--;

		sendBufferSum = 0;
		
		repeat = 1;

		goto init_send_loop;
	}

	return 1;
}

void send (ir_ncode *data,struct ir_remote *rem, int repeats)
{
	//=======================
	DWORD	mypriorityclass;
	DWORD	mythreadpriority;
	HANDLE	myprocess;
	HANDLE	mythread;
	//=======================

    if (!rem || !data) return;

	myprocess		= GetCurrentProcess();  //save these handles, because we'll need them several times
	mythread		= GetCurrentThread();
	mythreadpriority= GetPriorityClass(myprocess);  //store priority settings
    mypriorityclass	= GetThreadPriority(mythread);

	config_transmitter	(rem);
    SetPriorityClass	(myprocess,REALTIME_PRIORITY_CLASS);		//boost priority
    SetThreadPriority	(mythread,THREAD_PRIORITY_TIME_CRITICAL);

	init_timer			();
	init_send			(rem, data, repeats);

    SetPriorityClass	(myprocess,mypriorityclass); //restore original priorities
    SetThreadPriority	(mythread,mythreadpriority);
}


