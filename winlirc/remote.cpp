/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
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
 * Copyright (C) 1996,97 Ralph Metzler (rjkm@thp.uni-koeln.de)
 * Copyright (C) 1998 Christoph Bartelmus (columbus@hit.handshake.de)
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/timeb.h>

#include "remote.h"
#include "irdriver.h"
#include "learndlg.h"
#include "globals.h"


int gettimeofday(struct mytimeval *a, void *)
/* only accurate to milliseconds, instead of microseconds */
{
	struct _timeb tstruct;
	_ftime(&tstruct);
	
	a->tv_sec=tstruct.time;
	a->tv_usec=tstruct.millitm*1000;

	return 1;
}

struct ir_remote *get_ir_remote(char *name)
{
	CSingleLock lock(&CS_global_remotes,TRUE);

	struct ir_remote *all;

	/* use remotes carefully, it may be changed on SIGHUP */

	all=global_remotes;
	while(all)
	{
		if(strcasecmp(all->name,name)==0)
		{
			return(all);
		}
		all=all->next;
	}
	return(NULL);
}

struct ir_ncode *get_ir_code(struct ir_remote *remote,char *name)
{
	struct ir_ncode *all;

	all=remote->codes;
	while(all->name!=NULL)
	{
		if(strcasecmp(all->name,name)==0)
		{
			return(all);
		}
		all++;
	}
	return(0);
}

inline ir_code reverse(ir_code data,int bits)
{
	int i;
	ir_code c;
	
	c=0;
	for(i=0;i<bits;i++)
	{
		c|=(ir_code) (((data & (((ir_code) 1)<<i)) ? 1:0))
						     << (bits-1-i);
	}
	return(c);
}

inline void set_bit(ir_code *code,int bit,int data)
{
	(*code)&=~((((ir_code) 1)<<bit));
	(*code)|=((ir_code) (data ? 1:0)<<bit);
}

/*
  sending stuff

inline unsigned long time_left(struct mytimeval *current,struct mytimeval *last,
			       unsigned long gap)
{
	unsigned long secs,usecs,diff;
	
	secs=current->tv_sec-last->tv_sec;
	usecs=current->tv_usec-last->tv_usec;
	
	diff=1000000*secs+usecs;
	
	return(diff<gap ? gap-diff:0);
}

inline void clear_send_buffer(void)
{
	send_buffer.wptr=0;
	send_buffer.too_long=0;
	send_buffer.is_shift=0;
	send_buffer.pendingp=0;
	send_buffer.pendings=0;
	send_buffer.sum=0;
}

inline void add_send_buffer(unsigned long data)
{
	if(send_buffer.wptr<WBUF_SIZE)
	{
		send_buffer.sum+=data;
		send_buffer.data[send_buffer.wptr]=data;
		send_buffer.wptr++;
	}
	else
	{
		send_buffer.too_long=1;
	}
}

inline void send_pulse(unsigned long data)
{
	if(send_buffer.pendingp>0)
	{
		send_buffer.pendingp+=data;
	}
	else
	{
		if(send_buffer.pendings>0)
		{
			add_send_buffer(send_buffer.pendings);
			send_buffer.pendings=0;
		}
		send_buffer.pendingp=data;
	}
}

inline void send_space(unsigned long data)
{
	if(send_buffer.wptr==0 && send_buffer.pendingp==0)
	{
#ifdef __DEBUG
		logprintf("first signal is a space!\n");
#endif
		return;
	}
	if(send_buffer.pendings>0)
	{
		send_buffer.pendings+=data;
	}
	else
	{
		if(send_buffer.pendingp>0)
		{
			add_send_buffer(send_buffer.pendingp);
			send_buffer.pendingp=0;
		}
		send_buffer.pendings=data;
	}
}

inline int bad_send_buffer(void)
{
	if(send_buffer.too_long!=0) return(1);
	if(send_buffer.is_shift==1)
	{
		if(send_buffer.wptr==WBUF_SIZE && send_buffer.pendingp>0)
		{
			return(1);
		}
	}
	return(0);
}

int write_send_buffer(int length,unsigned long *signals)
{
#if defined(SIM_SEND) && !defined(DAEMONIZE)
	int i;

	if(send_buffer.wptr==0 && length>0 && signals!=NULL)
	{
		for(i=0;;)
		{
			printf("pulse %ld\n",signals[i++]);
			if(i>=length) break;
			printf("space %ld\n",signals[i++]);
		}
		return(length*sizeof(unsigned long));
	}
	
	if(send_buffer.pendingp>0)
	{
		add_send_buffer(send_buffer.pendingp);
		send_buffer.pendingp=0;
	}
	if(send_buffer.wptr==0) 
	{
#               ifdef __DEBUG
		logprintf("nothing to send\n");
#               endif __DEBUG
		return(0);
	}
	if(send_buffer.wptr%2==0) send_buffer.wptr--;
	for(i=0;;)
	{
		printf("pulse %ld\n",send_buffer.data[i++]);
		if(i>=send_buffer.wptr) break;
		printf("space %ld\n",send_buffer.data[i++]);
	}
	return(send_buffer.wptr*sizeof(unsigned long));
#else
	if(send_buffer.wptr==0 && length>0 && signals!=NULL)
	{
		return(write(lirc,signals,length*sizeof(unsigned long)));
	}
	
	if(send_buffer.pendingp>0)
	{
		add_send_buffer(send_buffer.pendingp);
		send_buffer.pendingp=0;
	}
	if(send_buffer.wptr==0) 
	{
#               ifdef __DEBUG
		logprintf("nothing to send\n");
#               endif __DEBUG
		return(0);
	}
	if(send_buffer.wptr%2==0) send_buffer.wptr--;
	return(write(lirc,send_buffer.data,
		     send_buffer.wptr*sizeof(unsigned long)));
#endif
}

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

inline void send_data(struct ir_remote *remote,ir_code data,int bits)
{
	int i;

	if(!(remote->flags&REVERSE)) data=reverse(data,bits);
	for(i=0;i<bits;i++)
	{
		if(data&1)
		{
			if(is_shift(remote))
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
			send_pulse(remote->pzero);
			send_space(remote->szero);
		}
		data=data>>1;
	}
}

inline void send_pre(struct ir_remote *remote)
{
	if(has_pre(remote))
	{
		ir_code pre;

		pre=remote->pre_data;
		if(remote->repeat_bit>0)
		{
			if(remote->repeat_bit<=remote->pre_data_bits)
			{
				set_bit(&pre,
					remote->pre_data_bits
					-remote->repeat_bit,
					remote->repeat_state);
			}
		}

		send_data(remote,pre,remote->pre_data_bits);
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
		ir_code post;

		post=remote->post_data;
		if(remote->repeat_bit>0)
		{
			if(remote->repeat_bit>remote->pre_data_bits
			   +remote->bits
			   &&
			   remote->repeat_bit<=remote->pre_data_bits
			   +remote->bits
			   +remote->post_data_bits)
			{
				set_bit(&post,
					remote->pre_data_bits
					+remote->bits
					+remote->post_data_bits
					-remote->repeat_bit,
					remote->repeat_state);
			}
		}
		
		if(remote->post_p>0 && remote->post_s>0)
		{
			send_pulse(remote->post_p);
			send_space(remote->post_s);
		}
		send_data(remote,post,remote->post_data_bits);
	}
}

inline void send_repeat(struct ir_remote *remote)
{
	send_lead(remote);
	send_pulse(remote->prepeat);
	send_space(remote->srepeat);
	send_trail(remote);
}

inline void send_code(struct ir_remote *remote,ir_code code)
{
	if(remote->repeat_bit>0)
	{
		if(remote->repeat_bit>remote->pre_data_bits
		   &&
		   remote->repeat_bit<=remote->pre_data_bits
		   +remote->bits)
		{
			set_bit(&code,
				remote->pre_data_bits
				+remote->bits
				-remote->repeat_bit,
				remote->repeat_state);
		}
		else if(remote->repeat_bit>remote->pre_data_bits
			+remote->bits
			+remote->post_data_bits)
		{
			logprintf("bad repeat_bit\n");
		}
	}

	if(repeat_remote==NULL || !(remote->flags&NO_HEAD_REP))
		send_header(remote);
	send_lead(remote);
	send_pre(remote);
	send_data(remote,code,remote->bits);
	send_post(remote);
	send_trail(remote);
	if(repeat_remote==NULL || !(remote->flags&NO_FOOT_REP))
		send_foot(remote);
}

void send_command(struct ir_remote *remote,struct ir_ncode *code)
{
	struct timeval current;
	unsigned long usecs;

	clear_send_buffer();
	if(is_shift(remote))
	{
		send_buffer.is_shift=1;
	}
	
	if(repeat_remote!=NULL && has_repeat(remote))
	{
		send_repeat(remote);
	}
	else
	{
		if(!is_raw(remote))
		{
			send_code(remote,code->code);
		}
	}
		
	if(bad_send_buffer())
	{
		logprintf("buffer too small\n");
		return;
	}

	gettimeofday(&current,NULL);

#if !defined(SIM_SEND) || defined(DAEMONIZE)
	if(remote->last_code!=NULL)
	{
		usecs=time_left(&current,&remote->last_send,
				remote->remaining_gap);
		if(usecs>0) usleep(usecs);
	}
#endif

	if(write_send_buffer(code->length,code->signals)==-1)
	{
		logprintf("write failed\n");
		logperror(NULL);
	}
	else
	{
		gettimeofday(&remote->last_send,NULL);
		if(is_const(remote))
		{
			remote->remaining_gap=remote->gap
			-send_buffer.sum;
		}
		else
		{
			if(has_repeat_gap(remote) && 
			   repeat_remote!=NULL && 
			   has_repeat(remote))
			{
				remote->remaining_gap=remote->repeat_gap;
			}
			else
			{
				remote->remaining_gap=remote->gap;
			}
		}
		remote->last_code=code;
#if defined(SIM_SEND) && !defined(DAEMONIZE)
		printf("space %ld\n",remote->remaining_gap);
#endif
	}
}

********************
SENDING STUFF done
********************/


/*
  decoding stuff
*/

void clear_rec_buffer(unsigned long data)
{
	int move;

#       ifdef __DEBUG2
	logprintf("c%ld\n",data&(PULSE_BIT-1));
#       endif
	move=rec_buffer.wptr-rec_buffer.rptr;
	if(move>0 && rec_buffer.rptr>0)
	{
		memmove(&rec_buffer.data[0],
			&rec_buffer.data[rec_buffer.rptr],
			sizeof(unsigned long)*move);
		rec_buffer.wptr-=rec_buffer.rptr;
	}
	else
	{
		rec_buffer.wptr=0;
	}
	rec_buffer.rptr=0;

	rec_buffer.data[rec_buffer.wptr]=data;
	rec_buffer.wptr++;

	rec_buffer.too_long=0;
	rec_buffer.is_shift=0;
	rec_buffer.pendingp=0;
	rec_buffer.pendings=0;
	rec_buffer.sum=0;
}

inline void rewind_rec_buffer()
{
	rec_buffer.rptr=0;
	rec_buffer.too_long=0;
	rec_buffer.pendingp=0;
	rec_buffer.pendings=0;
	rec_buffer.sum=0;
}

inline void unget_rec_buffer(int count)
{
	if(count==1 || count==2)
	{
		rec_buffer.rptr-=count;
		rec_buffer.sum-=rec_buffer.data[rec_buffer.rptr]&(PULSE_BIT-1);
		if(count==2)
		{
			rec_buffer.sum-=rec_buffer.data[rec_buffer.rptr+1]
			&(PULSE_BIT-1);
		}
	}
}

unsigned long get_next_rec_buffer(unsigned long maxusec)
{
	if(rec_buffer.rptr<rec_buffer.wptr)
	{
#               ifdef __DEBUG2
		logprintf("<%ld\n",rec_buffer.data[rec_buffer.rptr]
			  &(PULSE_BIT-1));
#               endif
		rec_buffer.sum+=rec_buffer.data[rec_buffer.rptr]&(PULSE_BIT-1);
		return(rec_buffer.data[rec_buffer.rptr++]);
	}
	else
	{
		if(rec_buffer.wptr<RBUF_SIZE)
		{
			if(use_ir_hardware)
				rec_buffer.data[rec_buffer.wptr]=ir_driver->readdata(maxusec,IRThreadEvent);
			else
				rec_buffer.data[rec_buffer.wptr]=learn_dialog->readdata(maxusec);
			if(rec_buffer.data[rec_buffer.wptr]==0) return(0);
			rec_buffer.sum+=rec_buffer.data[rec_buffer.rptr]
			&(PULSE_BIT-1);
			rec_buffer.wptr++;
			rec_buffer.rptr++;
#                       ifdef __DEBUG2
			logprintf("+%ld\n",rec_buffer.data[rec_buffer.rptr-1]
				  &(PULSE_BIT-1));
#                       endif
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

inline unsigned long get_next_pulse()
{
	unsigned long data;

	data=get_next_rec_buffer(1000*1000);
	if(data==0) return(0);
	if(!is_pulse(data))
	{
#               ifdef __DEBUG
		logprintf("pulse expected\n");
#               endif
		return(0);
	}
	return(data&(PULSE_BIT-1));
}

inline unsigned long get_next_space()
{
	unsigned long data;

	data=get_next_rec_buffer(1000*1000);
	if(data==0) return(0);
	if(!is_space(data))
	{
#               ifdef __DEBUG
		logprintf("space expected\n");
#               endif
		return(0);
	}
	return(data);
}

int expectpulse(struct ir_remote *remote,int exdelta)
{
	unsigned long deltas,deltap;
	int retval;

	if(rec_buffer.pendings>0)
	{
		deltas=get_next_space();
		if(deltas==0)
		{
			return(0);
		}
		retval=expect(remote,deltas,rec_buffer.pendings);
		if(!retval)
		{
			return(0);
		}
		rec_buffer.pendings=0;
	}
	
	deltap=get_next_pulse();
	if(deltap==0)
	{
		return(0);
	}
	if(rec_buffer.pendingp>0)
	{
		retval=expect(remote,deltap,
			      rec_buffer.pendingp+exdelta);
		if(!retval) 
		{ 	
			return(0);
		}
		rec_buffer.pendingp=0;
	}
	else
	{
		retval=expect(remote,deltap,exdelta);
	}
	return(retval);
}

int expectspace(struct ir_remote *remote,int exdelta)
{
	unsigned long deltas,deltap;
	int retval;

	if(rec_buffer.pendingp>0)
	{
		deltap=get_next_pulse();
		if(deltap==0) return(0);
		retval=expect(remote,deltap,rec_buffer.pendingp);
		if(!retval) return(0);
		rec_buffer.pendingp=0;
	}
	
	deltas=get_next_space();
	if(deltas==0) return(0);
	if(rec_buffer.pendings>0)
	{
		retval=expect(remote,deltas,
			      rec_buffer.pendings+exdelta);
		if(!retval) return(0);
		rec_buffer.pendings=0;
	}
	else
	{
		retval=expect(remote,deltas,exdelta);
	}
	return(retval);
}

inline int expectone(struct ir_remote *remote)
{
	if(is_shift(remote))
	{
		if(!expectspace(remote,remote->sone))
		{
			unget_rec_buffer(1);
			return(0);
		}
		rec_buffer.pendingp=remote->pone;
	}
	else
	{
		if(!expectpulse(remote,remote->pone))
		{
			unget_rec_buffer(1);
			return(0);
		}
		if(!expectspace(remote,remote->sone))
		{
			unget_rec_buffer(2);
			return(0);
		}
	}
	return(1);
}

inline int expectzero(struct ir_remote *remote)
{
	if(is_shift(remote))
	{
		if(!expectpulse(remote,remote->pzero))
		{
			unget_rec_buffer(1);
			return(0);
		}
		rec_buffer.pendings=remote->szero;
	}
	else
	{
		if(!expectpulse(remote,remote->pzero))
		{
			unget_rec_buffer(1);
			return(0);
		}
		if(!expectspace(remote,remote->szero))
		{
			unget_rec_buffer(2);
			return(0);
		}
	}
	return(1);
}

inline int sync_rec_buffer(struct ir_remote *remote)
{
	int count;
	unsigned long deltas,deltap;

	count=0;
	deltas=get_next_space();
	if(deltas==0) return(0);

	while(deltas<remote->remaining_gap*(100-remote->eps)/100
	      && deltas<remote->remaining_gap-remote->aeps)
	{
		deltap=get_next_pulse();
		if(deltap==0) return(0);
		deltas=get_next_space();
		if(deltas==0) return(0);
		count++;
		if(count>REC_SYNC) /* no sync found, 
				      let's try a diffrent remote */
		{
			return(0);
		}
	}
	rec_buffer.sum=0;
	return(deltas);
}

inline int get_header(struct ir_remote *remote)
{
	if(!expectpulse(remote,remote->phead)) return(0);
	rec_buffer.pendings=remote->shead;
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
	if(remote->plead==0) return(1);
	if(!expectpulse(remote,remote->plead)) return(0);
	return(1);	
}

inline int get_trail(struct ir_remote *remote)
{
	if(remote->ptrail!=0)
	{
		if(!expectpulse(remote,remote->ptrail)) return(0);
	}
	if(rec_buffer.pendingp>0)
	{
		if(!expectpulse(remote,0)) return(0);
	}
	return(1);
}

inline int get_gap(struct ir_remote *remote,unsigned long gap)
{
	unsigned long data;

	data=get_next_rec_buffer(gap*(100-remote->eps)/100);
	if(data==0) return(1);
	if(!is_space(data))
	{
#               ifdef __DEBUG
		logprintf("space expected\n");
#               endif
		return(0);
	}
#       ifdef __DEBUG2
	logprintf("sum: %ld\n",rec_buffer.sum);
#       endif
	if(data<gap*(100-remote->eps)/100 &&
	   data<gap-remote->aeps)
	{
#               ifdef __DEBUG
		logprintf("end of signal not found\n");
#               endif
		return(0);
	}
	else
	{
		unget_rec_buffer(1);
	}
	return(1);	
}

inline int get_repeat(struct ir_remote *remote)
{
	if(!get_lead(remote)) return(0);
	if(is_shift(remote))
	{
		if(!expectspace(remote,remote->srepeat)) return(0);
		if(!expectpulse(remote,remote->prepeat)) return(0);
	}
	else
	{
		if(!expectpulse(remote,remote->prepeat)) return(0);
		rec_buffer.pendings=remote->srepeat;
	}
	if(!get_trail(remote)) return(0);
	if(!get_gap(remote,
		    is_const(remote) ? 
		    remote->gap-rec_buffer.sum:
		    (has_repeat_gap(remote) ? remote->repeat_gap:remote->gap)
		    )) return(0);
	return(1);
}

ir_code get_data(struct ir_remote *remote,int bits)
{
	ir_code code;
	int i;

	code=0;

	for(i=0;i<bits;i++)
	{
		code=code<<1;
		if(expectone(remote))
		{
#                       ifdef __DEBUG2
			logprintf("1\n");
#                       endif
			code|=1;
		}
		else if(expectzero(remote))
		{
#                       ifdef __DEBUG2
			logprintf("0\n");
#                       endif
			code|=0;
		}
		else
		{
#                       ifdef __DEBUG
			logprintf("failed on bit %d\n",i+1);
#                       endif
			return((ir_code) -1);
		}
	}
	if(remote->flags&REVERSE) return(reverse(code,bits));
	return(code);
}

ir_code get_pre(struct ir_remote *remote)
{
	ir_code pre;

	pre=get_data(remote,remote->pre_data_bits);

	if(pre==(ir_code) -1)
	{
#               ifdef __DEBUG
		logprintf("failed on pre_data\n");
#               endif
		return((ir_code) -1);
	}
	if(remote->pre_p>0 && remote->pre_s>0)
	{
		if(!expectpulse(remote,remote->pre_p))
			return((ir_code) -1);
		rec_buffer.pendings=remote->pre_s;
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
		rec_buffer.pendings=remote->post_s;
	}

	post=get_data(remote,remote->post_data_bits);

	if(post==(ir_code) -1)
	{
#               ifdef __DEBUG
		logprintf("failed on post_data\n");
#               endif
		return((ir_code) -1);
	}
	return(post);
}

int decode(struct ir_remote *remote)
{
	ir_code pre,code,post,pre_mask=0,code_mask=0,post_mask=0;
	struct ir_ncode *codes,*found;
	int repeat_state;
	int sync;

	repeat_state=0; /* make compiler happy */
	code=pre=post=0;

	rec_buffer.is_shift=is_shift(remote) ? 1:0;

	/* we should get a long space first */
	if(!(sync=sync_rec_buffer(remote)))
	{
#               ifdef __DEBUG
		logprintf("failed on sync\n");
#               endif		
		return(0);
	}

#       ifdef __DEBUG
	logprintf("sync\n");
#       endif

	if(has_repeat(remote) && last_remote==remote)
	{
		if(get_repeat(remote))
		{
			if(remote->last_code==NULL)
			{
				;//logprintf("repeat code without last_code received\n");
				return(0);
			}

			remote->remaining_gap=
			is_const(remote) ? 
			remote->gap-rec_buffer.sum:
			(has_repeat_gap(remote) ?
			 remote->repeat_gap:remote->gap);
			remote->reps++;
			return(1);
		}
		else
		{
#                       ifdef __DEBUG
			logprintf("no repeat\n");
#                       endif
			rewind_rec_buffer();
			sync_rec_buffer(remote);
		}

	}

	if(has_header(remote))
	{
		if(!get_header(remote)
		   && !(remote->flags&NO_HEAD_REP && 
			(sync<=(int)(remote->gap+remote->gap*remote->eps/100)
			 || sync<=(int)(remote->gap+remote->aeps))))
		{
#                       ifdef __DEBUG
	 		logprintf("failed on header\n");
#                       endif
			return(0);
		}
#               ifdef __DEBUG
		logprintf("header\n");
#               endif
	}

	if(is_raw(remote))
	{
		struct ir_ncode *codes;
		int i;

		codes=remote->codes;
		found=NULL;
		while(codes->name!=NULL && found==NULL)
		{
			found=codes;
			for(i=0;i<codes->length;)
			{
				if(!expectpulse(remote,(int)codes->signals[i++]))
				{
					found=NULL;
					rewind_rec_buffer();
					sync_rec_buffer(remote);
					break;
				}
				if(i<codes->length &&
				   !expectspace(remote,(int)codes->signals[i++]))
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
				    remote->gap-rec_buffer.sum:
				    remote->gap)) 
				return(0);
		}
	}
	else
	{
		if(!get_lead(remote))
		{
#                       ifdef __DEBUG
			logprintf("failed on leading pulse\n");
#                       endif
			return(0);
		}
		
		if(has_pre(remote))
		{
			pre=get_pre(remote);
			if(pre==(ir_code) -1)
			{
#                               ifdef __DEBUG
				logprintf("failed on pre\n");
#                               endif
				return(0);
			}
#                       ifdef __DEBUG
#                       ifdef LONG_IR_CODE
			logprintf("pre: %llx\n",pre);
#                       else
			logprintf("pre: %lx\n",pre);
#                       endif
#                       endif
		}

		code=get_data(remote,remote->bits);
		if(code==(ir_code) -1)
		{
#                       ifdef __DEBUG
			logprintf("failed on code\n");
#                       endif
			return(0);
		}
#               ifdef __DEBUG
#               ifdef LONG_IR_CODE
		logprintf("code: %llx\n",code);
#               else
		logprintf("code: %lx\n",code);
#               endif		
#               endif

		if(has_post(remote))
		{
			post=get_post(remote);
			if(post==(ir_code) -1)
			{
#                               ifdef __DEBUG
				logprintf("failed on post\n");
#                               endif
				return(0);
			}
#                       ifdef __DEBUG
#                       ifdef LONG_IR_CODE
			logprintf("post: %llx\n",post);
#                       else
			logprintf("post: %lx\n",post);
#                       endif
#                       endif
		}
		if(!get_trail(remote))
		{
#                       ifdef __DEBUG
			logprintf("failed on trailing pulse\n");
#                       endif
			return(0);
		}
		if(has_foot(remote))
		{
			if(!get_foot(remote))
			{
#                               ifdef __DEBUG
				logprintf("failed on foot\n");
#                               endif		
				return(0);
			}
		}
		if(!get_gap(remote,
			    is_const(remote) ? 
			    remote->gap-rec_buffer.sum:
			    remote->gap)) 
			return(0);

		if(remote->repeat_bit>0)
		{
			if(remote->repeat_bit<=remote->pre_data_bits)
			{
				repeat_state=
				pre&(1<<(remote->pre_data_bits
					 -remote->repeat_bit)) ? 1:0;
				pre_mask=1<<(remote->pre_data_bits
					     -remote->repeat_bit);
			}
			else if(remote->repeat_bit<=remote->pre_data_bits
				+remote->bits)
			{
				repeat_state=
				code&(1<<(remote->pre_data_bits
					  +remote->bits
					  -remote->repeat_bit)) ? 1:0;
				code_mask=1<<(remote->pre_data_bits
					      +remote->bits
					      -remote->repeat_bit);
			}
			else if(remote->repeat_bit<=remote->pre_data_bits
				+remote->bits
				+remote->post_data_bits)
			{
				repeat_state=
				post&(1<<(remote->pre_data_bits
					  +remote->bits
					  +remote->post_data_bits
					  -remote->repeat_bit)) ? 1:0;
				post_mask=1<<(remote->pre_data_bits
					      +remote->bits
					      +remote->post_data_bits
					      -remote->repeat_bit);
			}
			else
			{
				;//logprintf("bad repeat_bit\n");
			}
		}

		if(has_pre(remote))
		{
			if((pre|pre_mask)!=(remote->pre_data|pre_mask))
			{
#                               ifdef __DEBUG
				logprintf("bad pre data\n");
#                               endif
#                               ifdef __DEBUG2
#                               ifdef LONG_IR_CODE
				logprintf("%llx %llx\n",pre,remote->pre_data);
#                               else
				logprintf("%lx %lx\n",pre,remote->pre_data);
#                               endif
#                               endif
				return(0);
			}
#                       ifdef __DEBUG
			logprintf("pre\n");
#                       endif
		}
	
		if(has_post(remote))
		{
			if((post|post_mask)!=(remote->post_data|post_mask))
			{
#                               ifdef __DEBUG
				logprintf("bad post data\n");
#                               endif
				return(0);
			}
#                       ifdef __DEBUG
			logprintf("post\n");
#                       endif
		}
		found=NULL;
		codes=remote->codes;
		if(codes!=NULL)
		{
			while(codes->name!=NULL)
			{
				if((codes->code|code_mask)==(code|code_mask))
				{
					found=codes;
					break;
				}
				codes++;
			}
		}
	}

	if(use_ir_hardware==false)
	{
		/* "nasty hack" for irrecord that I missed the first time around. :)  */
		remote->post_data=code;
		return 1;
	}
	else
	{
		if(found==NULL)	return(0);
		else
		{
			last_remote=remote;
			if(found==remote->last_code && !has_repeat(remote) && 
			   (!(remote->repeat_bit>0) || 
				repeat_state==remote->repeat_state))
			{
				if(sync<=(int)(remote->remaining_gap*(100+remote->eps)/100)
				   || sync<=(int)(remote->remaining_gap+remote->aeps))
					remote->reps++;
				else
					remote->reps=0;
			}
			else
			{
				remote->reps=0;
				remote->last_code=found;
				if(remote->repeat_bit>0)
				{
					remote->repeat_state=repeat_state;
				}
			}
			gettimeofday(&remote->last_send,NULL);
			if(is_const(remote))
			{
				remote->remaining_gap=remote->gap
				-rec_buffer.sum;
			}
			else
			{
				if(has_repeat_gap(remote) && 
				   repeat_remote!=NULL && 
				   has_repeat(remote))
				{
					remote->remaining_gap=remote->repeat_gap;
				}
				else
				{
					remote->remaining_gap=remote->gap;
				}

			}
			return(1);
		}
	}
}

char *decode_command(unsigned long data)
{
	CSingleLock lock(&CS_global_remotes,TRUE);
	
	struct ir_remote *all;
	static char message[PACKET_SIZE+1];

	clear_rec_buffer(data);

	/* use remotes carefully, it may be changed on SIGHUP */
	decoding=all=global_remotes;
	while(all)
	{
#               ifdef __DEBUG
		logprintf("trying \"%s\" remote\n",all->name);
#               endif
		
		if(decode(all))
		{
			int len;
			unsigned __int64 code;
			struct ir_remote *remote;

			remote=all;
			code=0;
			if(!(remote->flags&REVERSE))
			{
				if(has_pre(remote))
				{
					code|=remote->pre_data;
					code=code<<remote->bits;
				}
				code|=remote->last_code->code;
				if(has_post(remote))
				{
					code=code<<remote->post_data_bits;
					code|=remote->post_data;
				}
			}
			else
			{
				if(has_post(remote))
				{
					code|=remote->post_data;
					code=code<<remote->bits;
				}
				code|=remote->last_code->code;
				if(has_pre(remote))
				{
					code=code<<remote->pre_data_bits;
					code|=remote->pre_data;
				}
			}

#ifdef __GLIBC__
			/* It seems you can't print 64-bit longs on glibc */

			len=_snprintf(message,PACKET_SIZE,"%08lx%08lx %02x %s %s\n",
				     (unsigned long)
				     (code>>32),
				     (unsigned long)
				     (code&0xFFFFFFFF),
				     remote->reps,
				     remote->last_code->name,
				     remote->name);
#else
			len=_snprintf(message,PACKET_SIZE,"%016I64x %02x %s %s\n",
				     code,
				     remote->reps,
				     remote->last_code->name,
				     remote->name);
#endif
			decoding=NULL;
			if(len==-1)
			{
				DEBUG("Message buffer overflow\n");				
				return(NULL);
			}
			else
			{
				return(message);
			}
		}
		else
		{
#                       ifdef __DEBUG
			logprintf("failed \"%s\" remote\n",all->name);
#                       endif
			if(all->next!=NULL) rewind_rec_buffer();
		}
		all=all->next;
	}
	decoding=NULL;
	last_remote=NULL;
#       ifdef __DEBUG
	logprintf("decoding failed for all remotes\n");
#       endif __DEBUG
	return(NULL);
}
