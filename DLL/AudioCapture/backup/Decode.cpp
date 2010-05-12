#include "LIRCDefines.h"
#include <Windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include "Globals.h"

//=====================
struct rbuf rec_buffer;
struct ir_remote *last_remote = NULL;
//=====================

/* only accurate to milliseconds, instead of microseconds */
int gettimeofday(struct mytimeval *a, void *) {

	//====================
	struct _timeb tstruct;
	//====================

	_ftime(&tstruct);
	
	a->tv_sec	= tstruct.time;
	a->tv_usec	= tstruct.millitm*1000;

	return 1;
}

void rewind_rec_buffer()
{
	rec_buffer.rptr=0;
	rec_buffer.too_long=0;
	rec_buffer.pendingp=0;
	rec_buffer.pendings=0;
	rec_buffer.sum=0;
}

void unget_rec_buffer(int count)
{
	if(count==1 || count==2)
	{
		rec_buffer.rptr-=count;
		rec_buffer.sum-=rec_buffer.data[rec_buffer.rptr]&(PULSE_BIT-1); //&(PULSE_MASK) in LIRC 0.6.5
		if(count==2)
		{
			rec_buffer.sum-=rec_buffer.data[rec_buffer.rptr+1]
			&(PULSE_BIT-1); //&(PULSE_MASK) in LIRC 0.6.5
		}
	}
}

void clear_rec_buffer(unsigned long data)
{
	int move;

	move=rec_buffer.wptr-rec_buffer.rptr;

	if(move>0 && rec_buffer.rptr>0)
	{
		memmove(&rec_buffer.data[0], &rec_buffer.data[rec_buffer.rptr], sizeof(unsigned long)*move);
		rec_buffer.wptr-=rec_buffer.rptr;
	}
	else
	{
		rec_buffer.wptr=0;
	}

	rec_buffer.rptr=0;

	rec_buffer.data[rec_buffer.wptr]=data;
	rec_buffer.wptr++;

	rec_buffer.too_long		= 0;
	rec_buffer.is_biphase	= 0;
	rec_buffer.pendingp		= 0;
	rec_buffer.pendings		= 0;
	rec_buffer.sum			= 0;
}

unsigned long get_next_rec_buffer(unsigned long maxusec)
{
	if(rec_buffer.rptr<rec_buffer.wptr)
	{
		rec_buffer.sum+=rec_buffer.data[rec_buffer.rptr]&(PULSE_BIT-1);
		return(rec_buffer.data[rec_buffer.rptr++]);
	}
	else
	{
		if(rec_buffer.wptr<RBUF_SIZE)
		{
			//=================
			unsigned int data;
			//=================

			waitTillDataIsReady(maxusec);
			analyseAudio->getData(&data);

			//printf("%i data\n",data);

			rec_buffer.data[rec_buffer.wptr] = data;

			if(rec_buffer.data[rec_buffer.wptr]==0) return(0);
			rec_buffer.sum+=rec_buffer.data[rec_buffer.rptr] & (PULSE_BIT-1);
			rec_buffer.wptr++;
			rec_buffer.rptr++;

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

int get_gap(struct ir_remote *remote,unsigned long gap) {

	//=================
	unsigned long data;
	//=================

	data = get_next_rec_buffer(gap*(100-remote->eps)/100);

	if(data==0) return 1;

	if(!is_space(data))
	{
		return 0;
	}

	if(data<gap*(100-remote->eps)/100 && data<gap-remote->aeps)
	{
		return 0;
	}
	else
	{
		unget_rec_buffer(1);
	}

	return 1;	
}

unsigned long get_next_pulse() {

	//=================
	unsigned long data;
	//=================

	data = get_next_rec_buffer(1000*1000);

	if(data==0)			return(0);
	if(!is_pulse(data)) return(0);
	
	return (data&(PULSE_BIT-1));  //&(PULSE_MASK) in LIRC 0.6.5
}

unsigned long get_next_space() {

	//=================
	unsigned long data;
	//=================

	data=get_next_rec_buffer(1000*1000);

	if(data==0)			return 0;
	if(!is_space(data))	return 0;

	return data;
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

int get_foot(struct ir_remote *remote)
{
	if(!expectspace(remote,remote->sfoot)) return(0);
	if(!expectpulse(remote,remote->pfoot)) return(0);
	return(1);
}

int get_lead(struct ir_remote *remote)
{
	if(remote->plead==0) return(1);
	//if(!expectpulse(remote,remote->plead)) return(0);
	rec_buffer.pendingp=remote->plead;
	return(1);	
}

int get_trail(struct ir_remote *remote)
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

int get_header(struct ir_remote *remote)
{
	if(is_rcmm(remote))
	{
		int deltap,deltas,sum;
		deltap=get_next_pulse();
		if(deltap==0)
		{
			unget_rec_buffer(1);
			return(0);
		}
		deltas=get_next_space();
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
	if(!expectpulse(remote,remote->phead))
	{
		unget_rec_buffer(1);
		return(0);
	}
	if(!expectspace(remote,remote->shead))
	{
		unget_rec_buffer(2);
		return(0);
	}
	return(1);
}

inline int expectone(struct ir_remote *remote,int bit)
{
	if(is_biphase(remote))
	{
		if(is_rc6(remote) &&
		   remote->toggle_bit>0 &&
		   bit==remote->toggle_bit-1)
		{
			if(remote->sone>0 &&
			   !expectspace(remote,2*remote->sone))
			{
				unget_rec_buffer(1);
				return(0);
			}
			rec_buffer.pendingp=2*remote->pone;
		}
		else
		{
			if(remote->sone>0 && !expectspace(remote,remote->sone))
			{
				unget_rec_buffer(1);
				return(0);
			}
			rec_buffer.pendingp=remote->pone;
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
			rec_buffer.pendings=remote->sone;
		}
	}
	return(1);
}

inline int expectzero(struct ir_remote *remote,int bit)
{
	if(is_biphase(remote))
	{
		if(is_rc6(remote) &&
		   remote->toggle_bit>0 &&
		   bit==remote->toggle_bit-1)
		{
			if(!expectpulse(remote,2*remote->pzero))
			{
				unget_rec_buffer(1);
				return(0);
			}
			rec_buffer.pendings=2*remote->szero;
			
		}
		else
		{
			if(!expectpulse(remote,remote->pzero))
			{
				unget_rec_buffer(1);
				return(0);
			}
			rec_buffer.pendings=remote->szero;
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
			rec_buffer.pendings=remote->szero;
		}
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
		rec_buffer.pendings=remote->srepeat;
	}
	if(!get_trail(remote)) return(0);
	if(!get_gap(remote,
		    is_const(remote) ? 
			(remote->gap>rec_buffer.sum ? remote->gap-rec_buffer.sum:0):
		    (has_repeat_gap(remote) ? remote->repeat_gap:remote->gap)
		    )) return(0);
	return(1);
}

int sync_rec_buffer(struct ir_remote *remote)
{
	int count;
	unsigned long deltas,deltap;

	count=0;
	deltas=get_next_space();

	if(deltas==0) return(0);
	if(last_remote!=NULL && !is_rcmm(remote))
	{
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
	}
	rec_buffer.sum=0;
	return(deltas);
}

ir_code get_data(struct ir_remote *remote,int bits,int done)
{
	ir_code code;
	int i;

	code=0;

	if(is_rcmm(remote))
	{
		int deltap,deltas,sum;
		
		if(bits%2 || done%2)
		{
			return((ir_code) -1);
		}
		for(i=0;i<bits;i+=2)
		{
			code<<=2;
			deltap=get_next_pulse();
			deltas=get_next_space();
			if(deltap==0 || deltas==0) 
			{
#               ifdef __DEBUG
				logprintf(LOG_ERR,"failed on bit %d",
					  done+i+1);
#				endif
			}
			sum=deltap+deltas;

			if(expect(remote,sum,remote->pzero+remote->szero))
			{
				code|=0;
			}
			else if(expect(remote,sum,remote->pone+remote->sone))
			{
				code|=1;
			}
			else if(expect(remote,sum,remote->ptwo+remote->stwo))
			{
				code|=2;
			}
			else if(expect(remote,sum,remote->pthree+remote->sthree))
			{
				code|=3;
			}
			else
			{
				return((ir_code) -1);
			}
		}
		return(code);
	}

	for(i=0;i<bits;i++)
	{
		code=code<<1;
		if(expectone(remote,done+i))
		{
			code|=1;
		}
		else if(expectzero(remote,done+i))
		{
			code|=0;
		}
		else
		{
			return((ir_code) -1);
		}
	}
	if(remote->flags&REVERSE) return(reverse(code,bits));  //different than LIRC 0.6.5
	return(code);
}

ir_code get_pre(struct ir_remote *remote)
{
	ir_code pre;

	pre=get_data(remote,remote->pre_data_bits,0);

	if(pre==(ir_code) -1)
	{
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

	post=get_data(remote,remote->post_data_bits,remote->pre_data_bits+
				  remote->bits);

	if(post==(ir_code) -1)
	{
		return((ir_code) -1);
	}
	return(post);
}

int decode(struct ir_remote *remote) //this is a lot different than LIRC 0.6.5
{
	ir_code pre,code,post,pre_mask=0,code_mask=0,post_mask=0;
	struct ir_ncode *codes,*found;
	int repeat_state;
	int sync;
	int header;

	repeat_state	= 0; /* make compiler happy */
	code			= 0;
	post			= 0;
	pre				= 0;

	rec_buffer.is_biphase=is_biphase(remote) ? 1:0;

	/* we should get a long space first */
	if(!(sync=sync_rec_buffer(remote)))
	{
#               ifdef __DEBUG
		logprintf("failed on sync\n");
#               endif	
		//printf("failed on sync %i\n",sync);
		return(0);
	}

#       ifdef __DEBUG
	logprintf("sync\n");
#       endif

	if(has_repeat(remote) && last_remote==remote)
	{
		if(remote->flags&REPEAT_HEADER && has_header(remote))
		{
				if(!get_header(remote))
				{
				//	LOGPRINTF(1,"failed on repeat "
				//		  "header");
					printf("failed on repeat header");
					return(0);
				}
				//LOGPRINTF(1,"repeat header");
		}
		if(get_repeat(remote))
		{
			if(remote->last_code==NULL)
			{
				//logprintf("repeat code without last_code received\n");
				printf("repeat code without last_code received\n");
				return(0);
			}

			remote->remaining_gap=
			is_const(remote) ? 
			(remote->gap>rec_buffer.sum ?
			 remote->gap-rec_buffer.sum:0):
			(has_repeat_gap(remote) ?
			 remote->repeat_gap:remote->gap);
			remote->reps++;
			printf("setting repeat here\n");
			return(1);
		}
		else
		{
#                       ifdef __DEBUG
			logprintf("no repeat\n");
#                       endif
			//printf("no repeat\n");
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
				(sync<=(int)(remote->gap+remote->gap*remote->eps/100)
				|| sync<=(int)(remote->gap+remote->aeps))))
			{
#                       ifdef __DEBUG
	 			logprintf("failed on header\n");
#                       endif
				//printf("failed on header\n");
				return(0);
			}
#               ifdef __DEBUG
			logprintf("header\n");
#               endif
		}
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
		if(found==NULL) return(0);  //in LIRC 0.6.5
		code=found->code;			//in LIRC 0.6.5
	}
	else
	{
		if(!get_lead(remote))
		{
#                       ifdef __DEBUG
			logprintf("failed on leading pulse\n");
#                       endif
			printf("failed on leading pulse\n");
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
				//printf("failed on pre\n");
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

		code=get_data(remote,remote->bits,remote->pre_data_bits);
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
		if(header==1 && is_const(remote) &&
		   (remote->flags&NO_HEAD_REP))
		{
			rec_buffer.sum-=remote->phead+remote->shead;
		}
		if(is_rcmm(remote))
		{
			if(!get_gap(remote,1000)) //the 1000 worries me
				return(0);
		}
		else if(is_const(remote))
		{
			if(!get_gap(remote,
				    remote->gap>rec_buffer.sum ?
				    remote->gap-rec_buffer.sum:0))
				return(0);
		}
		else
		{
			if(!get_gap(remote,(has_repeat_gap(remote) ?
								remote->repeat_gap:remote->gap)))
				return(0);
		}
		if(remote->toggle_bit>0)
		{
			if(remote->toggle_bit<=remote->pre_data_bits)
			{
				repeat_state=
				pre&(1i64<<(remote->pre_data_bits
					 -remote->toggle_bit)) ? 1:0;
				pre_mask=1i64<<(remote->pre_data_bits
					     -remote->toggle_bit);
			}
			else if(remote->toggle_bit<=remote->pre_data_bits
				+remote->bits)
			{
				repeat_state=
				code&(1i64<<(remote->pre_data_bits
					  +remote->bits
					  -remote->toggle_bit)) ? 1:0;
				code_mask=1i64<<(remote->pre_data_bits
					      +remote->bits
					      -remote->toggle_bit);
			}
			else if(remote->toggle_bit<=remote->pre_data_bits
				+remote->bits
				+remote->post_data_bits)
			{
				repeat_state=
				post&(1i64<<(remote->pre_data_bits
					  +remote->bits
					  +remote->post_data_bits
					  -remote->toggle_bit)) ? 1:0;
				post_mask=1i64<<(remote->pre_data_bits
					      +remote->bits
					      +remote->post_data_bits
					      -remote->toggle_bit);
			}
			else
			{
				//logprintf("bad toggle_bit\n");
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

	/*
	if(use_ir_hardware==false)
	{
		// "nasty hack" for irrecord that I missed the first time around. :)
		remote->post_data=code;
		return 1;
	}
	else */
	{
		if(found==NULL)	return(0);
		else
		{
			last_remote = remote;

			if(found==remote->last_code && !has_repeat(remote) && 
			   (!(remote->toggle_bit>0) || 
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
				if(remote->toggle_bit>0)
				{
					remote->repeat_state=repeat_state;
				}
			}
			gettimeofday(&remote->last_send,NULL);
			if(is_const(remote))
			{
				remote->remaining_gap=remote->gap>rec_buffer.sum ?
					remote->gap-rec_buffer.sum:0;
			}
			else
			{
					remote->remaining_gap=remote->gap;
			}
			return(1);
		}
	}
}

bool decodeCommand(struct ir_remote *remote, char *out) {

	//========
	UINT data;
	//========

	analyseAudio->getData(&data);
	clear_rec_buffer(data);

	while(remote) {
		
		if(decode(remote)) {

			//===========
			int		len;
			UINT64	code;
			//===========

			len		= 0;
			code	= 0;

			if(!(remote->flags&REVERSE)) {

				if(has_pre(remote)) {
					code|= remote->pre_data;
					code = code<<remote->bits;
				}

				code|= remote->last_code->code;

				if(has_post(remote)) {
					code = code<<remote->post_data_bits;
					code|= remote->post_data;
				}
			}
			else
			{
				if(has_post(remote)) {
					code|= remote->post_data;
					code = code<<remote->bits;
				}

				code|= remote->last_code->code;

				if(has_pre(remote)) {
					code = code<<remote->pre_data_bits;
					code|= remote->pre_data;
				}
			}

			len = _snprintf(out,PACKET_SIZE,"%016I64x %02x %s %s\n",
					 code,
					 remote->reps,
					 remote->last_code->name,
					 remote->name);

			if(len==-1) {
				return false;
			}
			printf("buffer %i\n",rec_buffer.wptr);
			
			return true;
		}
		else {
			if(remote->next!=NULL) rewind_rec_buffer();
		}
		remote = remote->next;
	}

	last_remote = NULL;

	return false;
}
