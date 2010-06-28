#include <Windows.h>
#include "TiraDLL.h"
#include "LIRCDefines.h"
#include <stdio.h>
#include "Globals.h"

#define CODE_LENGTH 64

//=============
ir_code irCode;
//=============

struct ir_remote *decoding		= NULL;
struct ir_remote *last_remote	= NULL;

int WINAPI tiraCallbackFunction(const char * eventstring) {

	EnterCriticalSection(&criticalSection);

	sscanf(eventstring,"I64x",&irCode);

	LeaveCriticalSection(&criticalSection);

	SetEvent(dataReadyEvent);

	return TIRA_TRUE;
}

int map_code(struct ir_remote *remote,
	     ir_code *prep,ir_code *codep,ir_code *postp,
	     int pre_bits,ir_code pre,
	     int bits,ir_code code,
	     int post_bits,ir_code post)
{
	ir_code all;
	
	if(pre_bits+bits+post_bits!=
	   remote->pre_data_bits+remote->bits+remote->post_data_bits)
	{
		return(0);
	}
	all=(pre&gen_mask(pre_bits));
	all<<=bits;
	all|=(code&gen_mask(bits));
	all<<=post_bits;
	all|=(post&gen_mask(post_bits));
	
	*postp=(all&gen_mask(remote->post_data_bits));
	all>>=remote->post_data_bits;
	*codep=(all&gen_mask(remote->bits));
	all>>=remote->bits;
	*prep=(all&gen_mask(remote->pre_data_bits));
	
	return(1);
}

void map_gap(struct ir_remote *remote,
	     struct mytimeval *start, struct mytimeval *last,
	     lirc_t signal_length,
	     int *repeat_flagp,
	     lirc_t *min_remaining_gapp,
	     lirc_t *max_remaining_gapp)
{
	// Time gap (us) between a keypress on the remote control and
	// the next one.
	lirc_t gap;
	
	// Check the time gap between the last keypress and this one.
	if (start->tv_sec - last->tv_sec >= 2) {
		// Gap of 2 or more seconds: this is not a repeated keypress.
		*repeat_flagp = 0;
		gap = 0;
	} else {
		// Calculate the time gap in microseconds.
		gap = time_elapsed(last, start);
		if(expect_at_most(remote, gap, remote->max_remaining_gap))
		{
			// The gap is shorter than a standard gap
			// (with relative or aboslute tolerance): this
			// is a repeated keypress.
			*repeat_flagp = 1;
		}
		else
		{
			// Standard gap: this is a new keypress.
			*repeat_flagp = 0;
		}
	}
	
	// Calculate extimated time gap remaining for the next code.
	if (is_const(remote)) {
		// The sum (signal_length + gap) is always constant
		// so the gap is shorter when the code is longer.
		if (min_gap(remote) > signal_length) {
			*min_remaining_gapp = min_gap(remote) - signal_length;
			*max_remaining_gapp = max_gap(remote) - signal_length;
		} else {
			*min_remaining_gapp = 0;
			if(max_gap(remote) > signal_length)
			{
				*max_remaining_gapp = max_gap(remote) - signal_length;
			}
			else
			{
				*max_remaining_gapp = 0;
			}
		}
	} else {
		// The gap after the signal is always constant.
		// This is the case of Kanam Accent serial remote.
		*min_remaining_gapp = min_gap(remote);
		*max_remaining_gapp = max_gap(remote);
	}	
}

int tira_decode (struct ir_remote *remote, ir_code *prep, ir_code *codep,
		 ir_code *postp, int *repeat_flagp,
		 lirc_t *min_remaining_gapp,
		 lirc_t *max_remaining_gapp)
{
	//==========
	int success;
	//==========

	success = 0;

	EnterCriticalSection(&criticalSection);

	success = map_code(remote, prep, codep, postp,0, 0, CODE_LENGTH, irCode, 0, 0);

	LeaveCriticalSection(&criticalSection);

	if(!success) return 0;

	map_gap(remote, &start, &last, 0, repeat_flagp,min_remaining_gapp, max_remaining_gapp);
	
	return 1;
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
			return(0);
		}
	}
	
	if(has_post(remote))
	{
		if((post|post_mask)!=(remote->post_data|post_mask))
		{
			return(0);
		}
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
						codes->current=codes->next;
					}
					else
					{
						codes->current=
							codes->current->next;
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
				
				search = codes->next;
				if(search == NULL ||
					(codes->next != NULL && codes->current == NULL))
				{
					codes->current=NULL;
				}
				else
				{
					int sequence_match = 0;
					
					while(search != codes->current->next)
					{
						struct ir_code_node *prev, *next;
						int flag = 1;
						
						prev = NULL; /* means codes->code */
						next = search;
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
						search = search->next;
					}
					if(!sequence_match) codes->current = NULL;
				}
			}
			codes++;
		}
	}
#       ifdef DYNCODES
	if(!found_code)
	{
		if(remote->dyncodes[remote->dyncode].code!=code)
		{
			remote->dyncode++;
			remote->dyncode%=2;
		}
		remote->dyncodes[remote->dyncode].code=code;
		found=&(remote->dyncodes[remote->dyncode]);
		found_code=1;
	}
#       endif
	if(found_code && found!=NULL && has_toggle_mask(remote))
	{
		if(!(remote->toggle_mask_state%2))
		{
			remote->toggle_code=found;
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

unsigned long long set_code(struct ir_remote *remote,struct ir_ncode *found,
			    ir_code toggle_bit_mask_state,int repeat_flag,
			    lirc_t min_remaining_gap, lirc_t max_remaining_gap)
{
	unsigned long long code;
	struct mytimeval current;

	gettimeofday(&current,NULL);
	if(remote==last_remote &&
	   (found==remote->last_code || (found->next!=NULL && found->current!=NULL)) &&
	   repeat_flag &&
	   time_elapsed(&remote->last_send,&current)<1000000 &&
	   (!has_toggle_bit_mask(remote) || toggle_bit_mask_state==remote->toggle_bit_mask_state))
	{
		if(has_toggle_mask(remote))
		{
			remote->toggle_mask_state++;
			if(remote->toggle_mask_state==4)
			{
				remote->reps++;
				remote->toggle_mask_state=2;
			}
		}
		else if(found->current==NULL)
		{
			remote->reps++;
		}
	}
	else
	{
		if(found->next!=NULL && found->current==NULL)
		{
			remote->reps=1;
		}
		else
		{
			remote->reps=0;
		}
		if(has_toggle_mask(remote))
		{
			remote->toggle_mask_state=1;
			remote->toggle_code=found;
		}
		if(has_toggle_bit_mask(remote))
		{
			remote->toggle_bit_mask_state=toggle_bit_mask_state;
		}
	}
	last_remote=remote;
	if(found->current==NULL) remote->last_code=found;
	remote->last_send=current;
	remote->min_remaining_gap=min_remaining_gap;
	remote->max_remaining_gap=max_remaining_gap;
	
	code=0;
	if(has_pre(remote))
	{
		code|=remote->pre_data;
		code=code<<remote->bits;
	}
	code|=found->code;
	if(has_post(remote))
	{
		code=code<<remote->post_data_bits;
		code|=remote->post_data;
	}
	if(remote->flags&COMPAT_REVERSE)
	{
		/* actually this is wrong: pre, code and post should
		   be rotated separately but we have to stay
		   compatible with older software
		 */
		code=reverse(code,bit_count(remote));
	}
	return(code);
}

int write_message(char *buffer, size_t size, const char *remote_name,
		  const char *button_name, const char *button_suffix,
		  ir_code code, int reps)
{
	int len;
	
	len=_snprintf(buffer, size, "%016llx %02x %s%s %s\n",
		     code,
		     reps,
		     button_name, button_suffix,
		     remote_name);
	return len;
}

bool decodeCommand(struct ir_remote *remotes, char *out)
{
	struct ir_remote *remote;
	ir_code pre,code,post;
	struct ir_ncode *ncode;
	int repeat_flag;
	ir_code toggle_bit_mask_state;
	lirc_t min_remaining_gap, max_remaining_gap;
	struct ir_remote *scan;
	struct ir_ncode *scan_ncode;
	
	/* use remotes carefully, it may be changed on SIGHUP */
	decoding=remote=remotes;
	while(remote)
	{
		
		if(tira_decode(remote,&pre,&code,&post,&repeat_flag,
				  &min_remaining_gap, &max_remaining_gap) &&
		   (ncode=get_code(remote,pre,code,post,&toggle_bit_mask_state)))
		{
			int len;

			code=set_code(remote,ncode,toggle_bit_mask_state,
				      repeat_flag,
				      min_remaining_gap,
				      max_remaining_gap);
			if((has_toggle_mask(remote) &&
			    remote->toggle_mask_state%2) ||
			   ncode->current!=NULL)
			{
				decoding=NULL;
				return(NULL);
			}

			for(scan = decoding; scan != NULL; scan = scan->next)
			{
				for( scan_ncode = scan->codes; scan_ncode->name != NULL; scan_ncode++)
				{
					scan_ncode->current = NULL;
				}
			}
			if(is_xmp(remote))
			{
				remote->last_code->current = remote->last_code->next;
			}
			
			len = write_message(out, PACKET_SIZE+1,
					    remote->name,
					    remote->last_code->name, "", code,
					    remote->reps-(ncode->next ? 1:0));
			decoding=NULL;
			if(len>=PACKET_SIZE+1)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{

		}
		remote->toggle_mask_state=0;
		remote=remote->next;
	}
	decoding=NULL;
	last_remote=NULL;

	return false;
}
