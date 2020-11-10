#include "../winlirc/ir_remote.h"
#include "../winlirc/constants.h"

#include <winlirc/WLPluginAPI.h>

#include <chrono>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>

using namespace std::chrono;

extern char *progname;
extern hardware hw;
extern rbuf rec_buffer;

struct ir_remote *decoding		= NULL;
struct ir_remote *last_remote	= NULL;
struct ir_remote *repeat_remot	= NULL;
struct ir_ncode *repeat_code;


//=========================================================================================
struct ir_remote *emulation_data;
struct ir_ncode *next_code = NULL;
struct ir_ncode *current_code = NULL;
int current_index = 0;
int current_rep = 0;

lirc_t emulation_readdata(lirc_t timeout)
{
	static lirc_t sum = 0;
	lirc_t data = 0;

	if(current_code == NULL)
	{
		data = 1000000;
		if(next_code)
		{
			current_code = next_code;
		}
		else
		{
			current_code = emulation_data->codes;
		}
		current_rep = 0;
		sum = 0;
	}
	else
	{
		if(current_code->name == NULL)
		{
			fprintf(stderr, "%s: %s no data found\n",
				progname, emulation_data->name);
			data = 0;
		}
		if(current_index >= current_code->length())
		{
			if(next_code)
			{
				current_code = next_code;
			}
			else
			{
				current_rep++;
				if(current_rep > 2)
				{
					current_code++;
					current_rep = 0;
					data = 1000000;
				}
			}
			current_index = 0;
			if(current_code->name == NULL)
			{
				current_code = NULL;
				return emulation_readdata(timeout);
			}
			if(data == 0)
			{
				if(is_const(emulation_data))
				{
					data = emulation_data->gap - sum;
				}
				else
				{
					data = emulation_data->gap;
				}
			}

			sum = 0;
		}
		else
		{
			data = current_code->signals[current_index];
			if((current_index % 2) == 0)
			{
				data |= PULSE_BIT;
			}
			current_index++;
			sum += data&PULSE_MASK;
		}
	}
	/*
	printf("delivering: %c%u\n", data&PULSE_BIT ? 'p':'s',
	data&PULSE_MASK);
	*/
	return data;
}

inline void set_pending_pulse(lirc_t deltap)
{
	//LOGPRINTF(5, "pending pulse: %lu", deltap);
	rec_buffer.pendingp=deltap;
}

inline void set_pending_space(lirc_t deltas)
{
	//LOGPRINTF(5, "pending space: %lu", deltas);
	rec_buffer.pendings=deltas;
}

lirc_t get_next_rec_buffer(lirc_t maxusec)
{
	if(rec_buffer.rptr<rec_buffer.wptr)
	{
		//LOGPRINTF(3,"<%c%lu",rec_buffer.data[rec_buffer.rptr]&PULSE_BIT ?'p':'s', (unsigned long)rec_buffer.data[rec_buffer.rptr]&(PULSE_MASK));
		rec_buffer.sum+=rec_buffer.data[rec_buffer.rptr]&(PULSE_MASK);
		return(rec_buffer.data[rec_buffer.rptr++]);
	}
	else
	{
		if(rec_buffer.wptr<RBUF_SIZE)
		{
			lirc_t data;
			
			data=emulation_readdata(2*maxusec<100000 ? 100000:2*maxusec);
			if(!data)
			{
				return 0;
			}

            rec_buffer.data[rec_buffer.wptr]=data;
            if(rec_buffer.data[rec_buffer.wptr]==0) return(0);
            rec_buffer.sum+=rec_buffer.data[rec_buffer.rptr]&(PULSE_MASK);
            rec_buffer.wptr++;
            rec_buffer.rptr++;
			//LOGPRINTF(3,"+%c%lu",rec_buffer.data[rec_buffer.rptr-1]&PULSE_BIT ? 'p':'s', (unsigned long)rec_buffer.data[rec_buffer.rptr-1]&(PULSE_MASK));
			return(rec_buffer.data[rec_buffer.rptr-1]);
		}
		else
		{
			rec_buffer.too_long=1;
			return(0);
		}
	}
	return(0);
}

void init_rec_buffer(void)
{
	memset(&rec_buffer,0,sizeof(rec_buffer));
}

void rewind_rec_buffer(void)
{
	rec_buffer.rptr=0;
	rec_buffer.too_long=0;
	set_pending_pulse(0);
	set_pending_space(0);
	rec_buffer.sum=0;
}

int clear_rec_buffer(void) {
	
	if(hw.rec_mode==LIRC_MODE_LIRCCODE || hw.rec_mode==LIRC_MODE_CODE) {

		rec_buffer.decoded = hw.get_ir_code();
	}

	else  {

		//===========
		lirc_t	data;
		int		move;
		//===========
		
		move=rec_buffer.wptr-rec_buffer.rptr;
		if(move>0 && rec_buffer.rptr>0)
		{
			memmove(&rec_buffer.data[0],
				&rec_buffer.data[rec_buffer.rptr],
				sizeof(rec_buffer.data[0])*move);
			rec_buffer.wptr-=rec_buffer.rptr;
		}
		else
		{
			rec_buffer.wptr=0;
			data=emulation_readdata(0);
						
			rec_buffer.data[rec_buffer.wptr]=data;
			rec_buffer.wptr++;
		}
	}

	rewind_rec_buffer();
	rec_buffer.is_biphase=0;
	
	return(1);
}

inline void unget_rec_buffer(int count)
{
	//LOGPRINTF(5, "unget: %d", count);
	if(count==1 || count==2)
	{
		rec_buffer.rptr-=count;
		rec_buffer.sum-=rec_buffer.data[rec_buffer.rptr]&(PULSE_MASK);
		if(count==2)
		{
			rec_buffer.sum-=rec_buffer.data[rec_buffer.rptr+1]
			&(PULSE_MASK);
		}
	}
}

inline void unget_rec_buffer_delta(lirc_t delta)
{
	rec_buffer.rptr--;
	rec_buffer.sum-=delta&(PULSE_MASK);
	rec_buffer.data[rec_buffer.rptr]=delta;
}

inline lirc_t get_next_pulse(lirc_t maxusec)
{
	lirc_t data;

	data=get_next_rec_buffer(maxusec);
	if(data==0) return(0);
	if(!is_pulse(data))
	{
		//LOGPRINTF(2,"pulse expected");
		return(0);
	}
	return(data&(PULSE_MASK));
}

inline lirc_t get_next_space(lirc_t maxusec)
{
	lirc_t data;

	data=get_next_rec_buffer(maxusec);
	if(data==0) return(0);
	if(!is_space(data))
	{
		//LOGPRINTF(2,"space expected");
		return(0);
	}
	return(data);
}

inline int sync_pending_pulse(struct ir_remote *remote)
{
	if(rec_buffer.pendingp>0)
	{
		lirc_t deltap;
		
		deltap=get_next_pulse(rec_buffer.pendingp);
		if(deltap==0) return 0;
		if(!expect(remote,deltap,rec_buffer.pendingp)) return 0;
		set_pending_pulse(0);
	}
	return 1;
}

inline int sync_pending_space(struct ir_remote *remote)
{
	if(rec_buffer.pendings>0)
	{
		lirc_t deltas;
		
		deltas=get_next_space(rec_buffer.pendings);
		if(deltas==0) return 0;
		if(!expect(remote,deltas,rec_buffer.pendings)) return 0;
		set_pending_space(0);
	}
	return 1;
}

int expectpulse(struct ir_remote *remote,int exdelta)
{
	lirc_t deltap;
	int retval;
	
	//LOGPRINTF(5, "expecting pulse: %lu", exdelta);
	if(!sync_pending_space(remote)) return 0;
	
	deltap=get_next_pulse(rec_buffer.pendingp+exdelta);
	if(deltap==0) return(0);
	if(rec_buffer.pendingp>0)
	{
		if(rec_buffer.pendingp>deltap) return 0;
		retval=expect(remote,deltap-rec_buffer.pendingp,exdelta);
		if(!retval) return(0);
		set_pending_pulse(0);
	}
	else
	{
		retval=expect(remote,deltap,exdelta);
	}
	return(retval);
}

int expectspace(struct ir_remote *remote,int exdelta)
{
	lirc_t deltas;
	int retval;

	//LOGPRINTF(5, "expecting space: %lu", exdelta);
	if(!sync_pending_pulse(remote)) return 0;
	
	deltas=get_next_space(rec_buffer.pendings+exdelta);
	if(deltas==0) return(0);
	if(rec_buffer.pendings>0)
	{
		if(rec_buffer.pendings>deltas) return 0;
		retval=expect(remote,deltas-rec_buffer.pendings,exdelta);
		if(!retval) return(0);
		set_pending_space(0);
	}
	else
	{
		retval=expect(remote,deltas,exdelta);
	}
	return(retval);
}

inline int expectone(struct ir_remote *remote,int bit)
{
	if(is_biphase(remote))
	{
		int all_bits = bit_count(remote);
		ir_code mask;
		
		mask=((ir_code) 1)<<(all_bits-1-bit);
		if(mask&remote->rc6_mask)
		{
			if(remote->sone>0 &&
			   !expectspace(remote,2*remote->sone))
			{
				unget_rec_buffer(1);
				return(0);
			}
			set_pending_pulse(2*remote->pone);
		}
		else
		{
			if(remote->sone>0 && !expectspace(remote,remote->sone))
			{
				unget_rec_buffer(1);
				return(0);
			}
			set_pending_pulse(remote->pone);
		}
	}
	else if(is_space_first(remote))
	{
		if(remote->sone>0 && !expectspace(remote,remote->sone))
		{
			unget_rec_buffer(1);
			return(0);
		}
		if(remote->pone>0 && !expectpulse(remote,remote->pone))
		{
			unget_rec_buffer(2);
			return(0);
		}
	}
	else
	{
		if(remote->pone>0 && !expectpulse(remote,remote->pone))
		{
			unget_rec_buffer(1);
			return(0);
		}
		if(remote->ptrail>0)
		{
			if(remote->sone>0 &&
			   !expectspace(remote,remote->sone))
			{
				unget_rec_buffer(2);
				return(0);
			}
		}
		else
		{
			set_pending_space(remote->sone);
		}
	}
	return(1);
}

inline int expectzero(struct ir_remote *remote,int bit)
{
	if(is_biphase(remote))
	{
		int all_bits = bit_count(remote);
		ir_code mask;
		
		mask=((ir_code) 1)<<(all_bits-1-bit);
		if(mask&remote->rc6_mask)
		{
			if(!expectpulse(remote,2*remote->pzero))
			{
				unget_rec_buffer(1);
				return(0);
			}
			set_pending_space(2*remote->szero);
		}
		else
		{
			if(!expectpulse(remote,remote->pzero))
			{
				unget_rec_buffer(1);
				return(0);
			}
			set_pending_space(remote->szero);
		}
	}
	else if(is_space_first(remote))
	{
		if(remote->szero>0 && !expectspace(remote,remote->szero))
		{
			unget_rec_buffer(1);
			return(0);
		}
		if(remote->pzero>0 && !expectpulse(remote,remote->pzero))
		{
			unget_rec_buffer(2);
			return(0);
		}
	}
	else
	{
		if(!expectpulse(remote,remote->pzero))
		{
			unget_rec_buffer(1);
			return(0);
		}
		if(remote->ptrail>0)
		{
			if(!expectspace(remote,remote->szero))
			{
				unget_rec_buffer(2);
				return(0);
			}
		}
		else
		{
			set_pending_space(remote->szero);
		}
	}
	return(1);
}

inline lirc_t sync_rec_buffer(struct ir_remote *remote)
{
	int count;
	lirc_t deltas,deltap;
	
	count=0;
	deltas=get_next_space(1000000);
	if(deltas==0) return(0);
	
	if(last_remote!=NULL && !is_rcmm(remote))
	{
		while(!expect_at_least(last_remote, deltas,
				       last_remote->min_remaining_gap))
		{
			deltap=get_next_pulse(1000000);
			if(deltap==0) return(0);
			deltas=get_next_space(1000000);
			if(deltas==0) return(0);
			count++;
			if(count>REC_SYNC) /* no sync found, 
					      let's try a diffrent remote */
			{
				return(0);
			}
		}
		if(has_toggle_mask(remote))
		{
			if(!expect_at_most(last_remote, deltas,
					   last_remote->max_remaining_gap))
			{
				remote->toggle_mask_state=0;
				remote->toggle_code=NULL;
			}
			
		}
	}
	rec_buffer.sum=0;
	return(deltas);
}

inline int get_header(struct ir_remote *remote)
{
	if(is_rcmm(remote))
	{
		lirc_t deltap,deltas,sum;
		
		deltap=get_next_pulse(remote->phead);
		if(deltap==0)
		{
			unget_rec_buffer(1);
			return(0);
		}
		deltas=get_next_space(remote->shead);
		if(deltas==0)
		{
			unget_rec_buffer(2);
			return(0);
		}
		sum=deltap+deltas;
		if(expect(remote,sum,remote->phead+remote->shead))
		{
			return(1);
		}
		unget_rec_buffer(2);
		return(0);
	}
	else if(is_bo(remote))
	{
		if(expectpulse(remote, remote->pone) &&
		   expectspace(remote, remote->sone) &&
		   expectpulse(remote, remote->pone) &&
		   expectspace(remote, remote->sone) &&
		   expectpulse(remote, remote->phead) &&
		   expectspace(remote, remote->shead))
		{
			return 1;
		}
		return 0;
	}
	if(remote->shead==0)
	{
		if(!sync_pending_space(remote)) return 0;
		set_pending_pulse(remote->phead);
		return 1;
	}
	if(!expectpulse(remote,remote->phead))
	{
		unget_rec_buffer(1);
		return(0);
	}
	/* if this flag is set I need a decision now if this is really
           a header */
	if(remote->flags&NO_HEAD_REP)
	{
		lirc_t deltas;
		
		deltas=get_next_space(remote->shead);
		if(deltas!=0)
		{
			if(expect(remote,remote->shead,deltas))
			{
				return(1);
			}
			unget_rec_buffer(2);
			return(0);
		}
	}
	
	set_pending_space(remote->shead);
	return(1);
}

inline int get_foot(struct ir_remote *remote)
{
	if(!expectspace(remote,remote->sfoot)) return(0);
	if(!expectpulse(remote,remote->pfoot)) return(0);
	return(1);
}

inline int get_lead(struct ir_remote *remote)
{
	if(remote->plead==0) return 1;
	if(!sync_pending_space(remote)) return 0;
	set_pending_pulse(remote->plead);
	return 1;
}

inline int get_trail(struct ir_remote *remote)
{
	if(remote->ptrail!=0)
	{
		if(!expectpulse(remote,remote->ptrail)) return(0);
	}
	if(rec_buffer.pendingp>0)
	{
		if(!sync_pending_pulse(remote)) return(0);
	}
	return(1);
}

inline int get_gap(struct ir_remote *remote,lirc_t gap)
{
	lirc_t data;
	
	//LOGPRINTF(2,"sum: %d",rec_buffer.sum);
	data=get_next_rec_buffer(gap-gap*remote->eps/100);
	if(data==0) return(1);
	if(!is_space(data))
	{
		//LOGPRINTF(2,"space expected");
		return(0);
	}
	unget_rec_buffer(1);
	if(!expect_at_least(remote, data, gap))
	{
		//LOGPRINTF(1,"end of signal not found");
		return(0);
	}
	return(1);	
}

inline int get_repeat(struct ir_remote *remote)
{
	if(!get_lead(remote)) return(0);
	if(is_biphase(remote))
	{
		if(!expectspace(remote,remote->srepeat)) return(0);
		if(!expectpulse(remote,remote->prepeat)) return(0);
	}
	else
	{
		if(!expectpulse(remote,remote->prepeat)) return(0);
		set_pending_space(remote->srepeat);
	}
	if(!get_trail(remote)) return(0);
	if(!get_gap(remote,
		    is_const(remote) ? 
		    (min_gap(remote)>rec_buffer.sum ? min_gap(remote)-rec_buffer.sum:0):
		    (has_repeat_gap(remote) ? remote->repeat_gap:min_gap(remote))
		    )) return(0);
	return(1);
}

ir_code get_data(struct ir_remote *remote,int bits,int done)
{
	ir_code code;
	int i;
	
	code=0;
	
	if(is_rcmm(remote))
	{
		lirc_t deltap,deltas,sum;
		
		if(bits%2 || done%2)
		{
			return((ir_code) -1);
		}
		if(!sync_pending_space(remote)) return 0;
		for(i=0;i<bits;i+=2)
		{
			code<<=2;
			deltap=get_next_pulse(remote->pzero+remote->pone+
					      remote->ptwo+remote->pthree);
			deltas=get_next_space(remote->szero+remote->sone+
					      remote->stwo+remote->sthree);
			if(deltap==0 || deltas==0) 
			{
				return((ir_code) -1);
			}
			sum=deltap+deltas;
			//LOGPRINTF(3,"rcmm: sum %ld",(unsigned long) sum);
			if(expect(remote,sum,remote->pzero+remote->szero))
			{
				code|=0;
				//LOGPRINTF(2,"00");
			}
			else if(expect(remote,sum,remote->pone+remote->sone))
			{
				code|=1;
				//LOGPRINTF(2,"01");
			}
			else if(expect(remote,sum,remote->ptwo+remote->stwo))
			{
				code|=2;
				//LOGPRINTF(2,"10");
			}
			else if(expect(remote,sum,remote->pthree+remote->sthree))
			{
				code|=3;
				//LOGPRINTF(2,"11");
			}
			else
			{
				//LOGPRINTF(2,"no match for %d+%d=%d",deltap,deltas,sum);
				return((ir_code) -1);
			}
		}
		return(code);
	}
	else if(is_grundig(remote))
	{
		lirc_t deltap,deltas,sum;
		int state,laststate;
		
		if(bits%2 || done%2)
		{
			return((ir_code) -1);
		}
		if(!sync_pending_pulse(remote)) return ((ir_code) -1);
		for(laststate=state=-1,i=0;i<bits;)
		{
			deltas=get_next_space(remote->szero+remote->sone+
					      remote->stwo+remote->sthree);
			deltap=get_next_pulse(remote->pzero+remote->pone+
					      remote->ptwo+remote->pthree);
			if(deltas==0 || deltap==0) 
			{
				return((ir_code) -1);
			}
			sum=deltas+deltap;
			//LOGPRINTF(3,"grundig: sum %ld",(unsigned long) sum);
			if(expect(remote,sum,remote->szero+remote->pzero))
			{
				state=0;
				//LOGPRINTF(2,"2T");
			}
			else if(expect(remote,sum,remote->sone+remote->pone))
			{
				state=1;
				//LOGPRINTF(2,"3T");
			}
			else if(expect(remote,sum,remote->stwo+remote->ptwo))
			{
				state=2;
				//LOGPRINTF(2,"4T");
			}
			else if(expect(remote,sum,remote->sthree+remote->pthree))
			{
				state=3;
				//LOGPRINTF(2,"6T");
			}
			else
			{
				//LOGPRINTF(2,"no match for %d+%d=%d",deltas,deltap,sum);
				return((ir_code) -1);
			}
			if(state==3) /* 6T */
			{
				i+=2;code<<=2;state=-1;
				code|=0;
			}
			else if(laststate==2 && state==0) /* 4T2T */
			{
				i+=2;code<<=2;state=-1;
				code|=1;
			}
			else if(laststate==1 && state==1) /* 3T3T */
			{
				i+=2;code<<=2;state=-1;
				code|=2;
			}
			else if(laststate==0 && state==2) /* 2T4T */
			{
				i+=2;code<<=2;state=-1;
				code|=3;
			}
			else if(laststate==-1)
			{
				/* 1st bit */
			}
			else
			{
				return((ir_code) -1);
			}
			laststate=state;
		}
		return(code);
	}
	else if(is_serial(remote))
	{
		int received;
		int space, start_bit, stop_bit, parity_bit;
		int parity;
		lirc_t delta,origdelta,pending,expecting, gap_delta;
		lirc_t base, stop;
		lirc_t max_space, max_pulse;
		
		base=1000000/remote->baud;
		
		/* start bit */
		set_pending_pulse(base);
		
		received=0;
		space=(rec_buffer.pendingp==0); /* expecting space ? */
		start_bit=0;
		stop_bit=0;
		parity_bit=0;
		delta=origdelta=0;
		stop=base*remote->stop_bits/2;
		parity=0;
		gap_delta=0;
		
		max_space = remote->sone*remote->bits_in_byte+stop;
		max_pulse = remote->pzero*(1+remote->bits_in_byte);
		if(remote->parity != IR_PARITY_NONE)
		{
			parity_bit = 1;
			max_space += remote->sone;
			max_pulse += remote->pzero;
			bits += bits/remote->bits_in_byte;
		}
		
		while(received<bits || stop_bit)
		{
			if(delta==0)
			{
				delta=space ?
					get_next_space(max_space):
					get_next_pulse(max_pulse);
				if(delta==0 && space &&
				   received+remote->bits_in_byte+parity_bit>=bits)
				{
					/* open end */
					delta=max_space;
				}
				origdelta=delta;
			}
			if(delta==0)
			{
				//LOGPRINTF(1,"failed before bit %d",received+1);
				return((ir_code) -1);
			}
			pending=(space ?
				 rec_buffer.pendings:rec_buffer.pendingp);
			if(expect(remote, delta, pending))
			{
				delta=0;
			}
			else if(delta>pending)
			{
				delta-=pending;
			}
			else
			{
				//LOGPRINTF(1,"failed before bit %d",received+1);
				return((ir_code) -1);
			}
			if(pending>0)
			{
				if(stop_bit)
				{
					//LOGPRINTF(5, "delta: %lu", delta);
					gap_delta = delta;
					delta=0;
					set_pending_pulse(base);
					set_pending_space(0);
					stop_bit=0;
					space=0;
					//LOGPRINTF(3,"stop bit found");
				}
				else
				{
					//LOGPRINTF(3,"pending bit found");
					set_pending_pulse(0);
					set_pending_space(0);
					if(delta==0)
					{
						space=(space ? 0:1);
					}
				}
				continue;
			}
			expecting=(space ? remote->sone:remote->pzero);
			if(delta>expecting || expect(remote,delta,expecting))
			{
				delta-=(expecting>delta ? delta:expecting);
				received++;
				code<<=1;
				code|=space;
				parity^=space;
				//LOGPRINTF(2,"adding %d",space);
				if(received%(remote->bits_in_byte+parity_bit)==0)
				{
					ir_code temp;
					
					if((remote->parity == IR_PARITY_EVEN && parity) ||
					   (remote->parity == IR_PARITY_ODD && !parity))
					{
						//LOGPRINTF(1, "parity error ""after %d bits",received+1);
						return((ir_code) -1);
					}
					parity = 0;
					
					/* parity bit is filtered out */
					temp=code>>(remote->bits_in_byte+parity_bit);
					code=temp<<remote->bits_in_byte|
						reverse(code>>parity_bit,
							remote->bits_in_byte);
					
					if(space && delta==0)
					{
						//LOGPRINTF(1,"failed at stop ""bit after %d bits",received+1);
						return((ir_code) -1);
					}
					//LOGPRINTF(3,"awaiting stop bit");
					set_pending_space(stop);
					stop_bit=1;
				}				
			}
			else
			{
				if(delta==origdelta)
				{
					//LOGPRINTF(1,"framing error after ""%d bits",received+1);
					return((ir_code) -1);
				}
				delta=0;
			}
			if(delta==0)
			{
				space=(space ? 0:1);
			}
		}
		if(gap_delta) unget_rec_buffer_delta(gap_delta);
		set_pending_pulse(0);
		set_pending_space(0);
		return code;
	}
	else if(is_bo(remote))
	{
		int lastbit = 1;
		lirc_t deltap,deltas;
		lirc_t pzero,szero;
		lirc_t pone,sone;
		
		for(i=0; i<bits; i++)
		{
			code<<=1;
			deltap=get_next_pulse(remote->pzero+remote->pone+
					      remote->ptwo+remote->pthree);
			deltas=get_next_space(remote->szero+remote->sone+
					      remote->stwo+remote->sthree);
			if(deltap==0 || deltas==0) 
			{
				return((ir_code) -1);
			}
			if(lastbit == 1)
			{
				pzero = remote->pone;
				szero = remote->sone;
				pone = remote->ptwo;
				sone = remote->stwo;
			}
			else
			{
				pzero = remote->ptwo;
				szero = remote->stwo;
				pone = remote->pthree;
				sone = remote->sthree;
			}
			//LOGPRINTF(5, "%lu %lu %lu %lu", pzero, szero, pone, sone);
			if(expect(remote, deltap, pzero))
			{
				if(expect(remote, deltas, szero))
				{
					code|=0;
					lastbit=0;
					//LOGPRINTF(2,"0");
					continue;
				}
			}
			
			if(expect(remote, deltap, pone))
			{
				if(expect(remote, deltas, sone))
				{
					code|=1;
					lastbit=1;
					//LOGPRINTF(2,"1");
					continue;
				}
			}

			return((ir_code) -1);
		}
		return code;
	}
	else if(is_xmp(remote))
	{
		lirc_t deltap,deltas,sum;
		ir_code n;
		
		if(bits%4 || done%4)
		{
			return((ir_code) -1);
		}
		if(!sync_pending_space(remote)) return 0;
		for(i=0;i<bits;i+=4)
		{
			code<<=4;
			deltap=get_next_pulse(remote->pzero);
			deltas=get_next_space(remote->szero+16*remote->sone);
			if(deltap==0 || deltas==0) 
			{
				return((ir_code) -1);
			}
			sum=deltap+deltas;
			
			sum -= remote->pzero + remote->szero;
			n = (sum + remote->sone/2)/remote->sone;
			if(n >= 16)
			{
				return((ir_code) -1);
			}
			//LOGPRINTF(1, "%d: %lx", i, n);
			code |= n;

		}
		return code;
	}
	
	for(i=0;i<bits;i++)
	{
		code=code<<1;
		if(is_goldstar(remote))
		{
			if((done+i)%2)
			{
				//LOGPRINTF(2,"$1");
				remote->pone=remote->ptwo;
				remote->sone=remote->stwo;
			}
			else
			{
				//LOGPRINTF(2,"$2");
				remote->pone=remote->pthree;
				remote->sone=remote->sthree;
			}
		}
		
		if(expectone(remote,done+i))
		{
			//LOGPRINTF(2,"1");
			code|=1;
		}
		else if(expectzero(remote,done+i))
		{
			//LOGPRINTF(2,"0");
			code|=0;
		}
		else
		{
			//LOGPRINTF(1,"failed on bit %d",done+i+1);
			return((ir_code) -1);
		}
	}
	return(code);
}

ir_code get_pre(struct ir_remote *remote)
{
	ir_code pre;

	pre=get_data(remote,remote->pre_data_bits,0);

	if(pre==(ir_code) -1)
	{
		//LOGPRINTF(1,"failed on pre_data");
		return((ir_code) -1);
	}
	if(remote->pre_p>0 && remote->pre_s>0)
	{
		if(!expectpulse(remote,remote->pre_p))
			return((ir_code) -1);
		set_pending_space(remote->pre_s);
	}
	return(pre);
}

ir_code get_post(struct ir_remote *remote)
{
	ir_code post;

	if(remote->post_p>0 && remote->post_s>0)
	{
		if(!expectpulse(remote,remote->post_p))
			return((ir_code) -1);
		set_pending_space(remote->post_s);
	}

	post=get_data(remote,remote->post_data_bits,remote->pre_data_bits+
		      remote->bits);

	if(post==(ir_code) -1)
	{
		//LOGPRINTF(1,"failed on post_data");
		return((ir_code) -1);
	}
	return(post);
}

int receive_decode(struct ir_remote *remote,
		   ir_code *prep,ir_code *codep,ir_code *postp,
		   int *repeat_flagp,
		   lirc_t *min_remaining_gapp, lirc_t *max_remaining_gapp)
{
	ir_code pre,code,post;
	lirc_t sync;
	int header;
	steady_clock::time_point current;

	sync=0; /* make compiler happy */
	code=pre=post=0;
	header=0;

	if(hw.rec_mode==LIRC_MODE_MODE2 ||
	   hw.rec_mode==LIRC_MODE_PULSE ||
	   hw.rec_mode==LIRC_MODE_RAW)
	{
		rewind_rec_buffer();
		rec_buffer.is_biphase=is_biphase(remote) ? 1:0;
		
		/* we should get a long space first */
		if(!(sync=sync_rec_buffer(remote)))
		{
			//LOGPRINTF(1,"failed on sync");
			return(0);
		}

		//LOGPRINTF(1,"sync");

		if(has_repeat(remote) && last_remote==remote)
		{
			if(remote->flags&REPEAT_HEADER && has_header(remote))
			{
				if(!get_header(remote))
				{
					//LOGPRINTF(1,"failed on repeat ""header");
					return(0);
				}
				//LOGPRINTF(1,"repeat header");
			}
			if(get_repeat(remote))
			{
				if(remote->last_code==NULL)
				{
					return(0);
				}

				*prep=remote->pre_data;
				*codep=remote->last_code->code;
				*postp=remote->post_data;
				*repeat_flagp=1;

				*min_remaining_gapp=
					is_const(remote) ? 
					(min_gap(remote)>rec_buffer.sum ?
					 min_gap(remote)-rec_buffer.sum:0):
					(has_repeat_gap(remote) ?
					 remote->repeat_gap:min_gap(remote));
				*max_remaining_gapp=
					is_const(remote) ? 
					(max_gap(remote)>rec_buffer.sum ?
					 max_gap(remote)-rec_buffer.sum:0):
					(has_repeat_gap(remote) ?
					 remote->repeat_gap:max_gap(remote));
				return(1);
			}
			else
			{
				//LOGPRINTF(1,"no repeat");
				rewind_rec_buffer();
				sync_rec_buffer(remote);
			}

		}

		if(has_header(remote))
		{
			header=1;
			if(!get_header(remote))
			{
				header=0;
				if(!(remote->flags&NO_HEAD_REP && 
				     expect_at_most(remote,sync,max_gap(remote))))
				{
					//LOGPRINTF(1,"failed on header");
					return(0);
				}
			}
			//LOGPRINTF(1,"header");
		}
	}

	if(is_raw(remote))
	{
		struct ir_ncode *codes,*found;
		int i;

		if(hw.rec_mode==LIRC_MODE_CODE ||
		   hw.rec_mode==LIRC_MODE_LIRCCODE)
			return(0);

		codes=remote->codes;
		found=NULL;
		while(codes->name!=NULL && found==NULL)
		{
			found=codes;
			for(i=0;i<codes->length();)
			{
				if(!expectpulse(remote,codes->signals[i++]))
				{
					found=NULL;
					rewind_rec_buffer();
					sync_rec_buffer(remote);
					break;
				}
				if(i<codes->length() &&
				   !expectspace(remote,codes->signals[i++]))
				{
					found=NULL;
					rewind_rec_buffer();
					sync_rec_buffer(remote);
					break;
				}
			}
			codes++;
		}
		if(found!=NULL)
		{
			if(!get_gap(remote,
				    is_const(remote) ? 
				    min_gap(remote)-rec_buffer.sum:
				    min_gap(remote))) 
				found=NULL;
		}
		if(found==NULL) return(0);
		code=found->code;
	}
	else
	{
		if(hw.rec_mode==LIRC_MODE_CODE ||
		   hw.rec_mode==LIRC_MODE_LIRCCODE)
		{
 			lirc_t sum;
			ir_code decoded = rec_buffer.decoded;

			//LOGPRINTF(1,"decoded: %llx", decoded);
			if((hw.rec_mode==LIRC_MODE_CODE &&
			    hw.code_length<bit_count(remote))
			   ||
			   (hw.rec_mode==LIRC_MODE_LIRCCODE && 
			    hw.code_length!=bit_count(remote)))
			{
				return(0);
			}
			
			post = decoded & gen_mask(remote->post_data_bits);
			decoded >>= remote->post_data_bits;
			code = decoded & gen_mask(remote->bits);
			pre = decoded >> remote->bits;
			
			current = steady_clock::now();
			sum=remote->phead+remote->shead+
				std::max(remote->pone+remote->sone,
					   remote->pzero+remote->szero)*
				bit_count(remote)+
				remote->plead+
				remote->ptrail+
				remote->pfoot+remote->sfoot+
				remote->pre_p+remote->pre_s+
				remote->post_p+remote->post_s;
			
			rec_buffer.sum=sum>=remote->gap ? remote->gap-1:sum;
			sync = duration_cast<microseconds>(current - remote->last_send - microseconds{ rec_buffer.sum }).count();
		}
		else
		{
			if(!get_lead(remote))
			{
				//LOGPRINTF(1,"failed on leading pulse");
				return(0);
			}
			
			if(has_pre(remote))
			{
				pre=get_pre(remote);
				if(pre==(ir_code) -1)
				{
					//LOGPRINTF(1,"failed on pre");
					return(0);
				}
				//LOGPRINTF(1,"pre: %llx",pre);
			}
			
			code=get_data(remote,remote->bits,
				      remote->pre_data_bits);
			if(code==(ir_code) -1)
			{
				//LOGPRINTF(1,"failed on code");
				return(0);
			}
			//LOGPRINTF(1,"code: %llx",code);
			
			if(has_post(remote))
			{
				post=get_post(remote);
				if(post==(ir_code) -1)
				{
					//LOGPRINTF(1,"failed on post");
					return(0);
				}
				//LOGPRINTF(1,"post: %llx",post);
			}
			if(!get_trail(remote))
			{
				//LOGPRINTF(1,"failed on trailing pulse");
				return(0);
			}
			if(has_foot(remote))
			{
				if(!get_foot(remote))
				{
					//LOGPRINTF(1,"failed on foot");
					return(0);
				}
			}
			if(header==1 && is_const(remote) &&
			   (remote->flags&NO_HEAD_REP))
			{
				rec_buffer.sum-=remote->phead+remote->shead;
			}
			if(is_rcmm(remote))
			{
				if(!get_gap(remote,1000))
					return(0);
			}
			else if(is_const(remote))
			{
				if(!get_gap(remote,
					    min_gap(remote)>rec_buffer.sum ?
					    min_gap(remote)-rec_buffer.sum:0))
					return(0);
			}
			else
			{
				if(!get_gap(remote, min_gap(remote)))
					return(0);
			}
		} /* end of mode specific code */
	}
	*prep=pre;*codep=code;*postp=post;
	if((!has_repeat(remote) || remote->reps<remote->min_code_repeat) &&
	   expect_at_most(remote, sync, remote->max_remaining_gap))
		*repeat_flagp=1;
	else
		*repeat_flagp=0;
	if(hw.rec_mode==LIRC_MODE_CODE ||
	   hw.rec_mode==LIRC_MODE_LIRCCODE)
	{
		/* Most TV cards don't pass each signal to the
                   driver. This heuristic should fix repeat in such
                   cases. */
		if(current - remote->last_send < 325000us)
		{
			*repeat_flagp=1;
		}
	}
	if(is_const(remote))
	{
		*min_remaining_gapp=min_gap(remote)>rec_buffer.sum ?
			min_gap(remote)-rec_buffer.sum:0;
		*max_remaining_gapp=max_gap(remote)>rec_buffer.sum ?
			max_gap(remote)-rec_buffer.sum:0;
	}
	else
	{
		*min_remaining_gapp=min_gap(remote);
		*max_remaining_gapp=max_gap(remote);
	}
	return(1);
}

struct ir_ncode *get_code(struct ir_remote *remote,
			  ir_code pre,ir_code code,ir_code post,
			  ir_code *toggle_bit_mask_statep)
{
	ir_code pre_mask,code_mask,post_mask,toggle_bit_mask_state,all;
	int found_code, have_code;
	struct ir_ncode *codes,*found;
	
	pre_mask=code_mask=post_mask=0;

	if(has_toggle_bit_mask(remote))
	{
		pre_mask = remote->toggle_bit_mask >>
			   (remote->bits + remote->post_data_bits);
		post_mask = remote->toggle_bit_mask &
		            gen_mask(remote->post_data_bits);
	}
	if(has_ignore_mask(remote))
	{
		pre_mask |= remote->ignore_mask >>
			   (remote->bits + remote->post_data_bits);
		post_mask |= remote->ignore_mask &
		            gen_mask(remote->post_data_bits);
	}
	if(has_toggle_mask(remote) && remote->toggle_mask_state%2)
	{
		ir_code *affected,mask,mask_bit;
		int bit,current_bit;
		
		affected=&post;
		mask=remote->toggle_mask;
		for(bit=current_bit=0;bit<bit_count(remote);bit++,current_bit++)
		{
			if(bit==remote->post_data_bits)
			{
				affected=&code;
				current_bit=0;
			}
			if(bit==remote->post_data_bits+remote->bits)
			{
				affected=&pre;
				current_bit=0;
			}
			mask_bit=mask&1;
			(*affected)^=(mask_bit<<current_bit);
			mask>>=1;
		}
	}
	if(has_pre(remote))
	{
		if((pre|pre_mask)!=(remote->pre_data|pre_mask))
		{
			//LOGPRINTF(1,"bad pre data");
			//LOGPRINTF(2,"%llx %llx",pre,remote->pre_data);
			return(0);
		}
		//LOGPRINTF(1,"pre");
	}
	
	if(has_post(remote))
	{
		if((post|post_mask)!=(remote->post_data|post_mask))
		{
			//LOGPRINTF(1,"bad post data");
			//LOGPRINTF(2,"%llx %llx",post,remote->post_data);
			return(0);
		}
		//LOGPRINTF(1,"post");
	}

	all = gen_ir_code(remote, pre, code, post);

	toggle_bit_mask_state = all&remote->toggle_bit_mask;

	found=NULL;
	found_code=0;
	have_code=0;
	codes=remote->codes;
	if(codes!=NULL)
	{
		while(codes->name!=NULL)
		{
			ir_code next_all;

			next_all = gen_ir_code(remote, remote->pre_data,
					       get_ir_code(codes, codes->current),
					       remote->post_data);
			if(match_ir_code(remote, next_all, all))
			{
				found_code=1;
				if(codes->next!=NULL)
				{
					if(codes->current==NULL)
					{
						codes->current=codes->next.get();
					}
					else
					{
						codes->current = codes->current->next.get();
					}
				}
				if(!have_code)
				{
					found=codes;
					if(codes->current==NULL)
					{
						have_code=1;
					}
				}
			}
			else
			{
				/* find longest matching sequence */
				struct ir_code_node *search;
				
				search = codes->next.get();
				if(search == nullptr ||
					codes->next != nullptr && codes->current == nullptr)
				{
					codes->current = nullptr;
				}
				else
				{
					int sequence_match = 0;
					while(search != codes->current->next.get())
					{
						int flag = 1;
						
						ir_code_node* prev = nullptr; /* means codes->code */
						ir_code_node* next = search;
						while(next != codes->current)
						{
							if(get_ir_code(codes, prev) != get_ir_code(codes, next))
							{
								flag = 0;
								break;
							}
							prev = get_next_ir_code_node(codes, prev);
							next = get_next_ir_code_node(codes, next);
						}
						if(flag == 1)
						{
							next_all = gen_ir_code(remote, remote->pre_data,
									       get_ir_code(codes, prev),
									       remote->post_data);
							if(match_ir_code(remote, next_all, all))
							{
								codes->current = get_next_ir_code_node(codes, prev);
								sequence_match = 1;
								found_code=1;
								found=codes;
								break;
							}
						}
						search = search->next.get();
					}
					if (!sequence_match) codes->current = nullptr;
				}
			}
			codes++;
		}
	}

	if(found_code && found!=NULL && has_toggle_mask(remote))
	{
		if(!(remote->toggle_mask_state%2))
		{
			remote->toggle_code=found;
			//LOGPRINTF(1,"toggle_mask_start");
		}
		else
		{
			if(found!=remote->toggle_code)
			{
				remote->toggle_code=NULL;
				return(NULL);
			}
			remote->toggle_code=NULL;
		}
	}
	*toggle_bit_mask_statep=toggle_bit_mask_state;
	return(found);
}