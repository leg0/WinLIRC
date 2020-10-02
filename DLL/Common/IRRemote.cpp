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

#include "IRRemote.h"
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include "LIRCDefines.h"
#include "Hardware.h"
#include <limits.h>

using namespace std::chrono_literals;

struct ir_remote *decoding		= nullptr;
struct ir_remote *last_remote	= nullptr;
struct ir_remote *repeat_remote	= nullptr;
struct ir_ncode *repeat_code	= nullptr;

WINLIRC_API int map_code(struct ir_remote *remote,
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

WINLIRC_API void map_gap(
		ir_remote *remote,
		int64_t time_elapsed_us,
		lirc_t signal_length,
		int *repeat_flagp,
		lirc_t *min_remaining_gapp,
		lirc_t *max_remaining_gapp)
{
	// Time gap (us) between a keypress on the remote control and
	// the next one.
	lirc_t gap;
	
	// Check the time gap between the last keypress and this one.
	if (time_elapsed_us >= 2'000'000) {
		// Gap of 2 or more seconds: this is not a repeated keypress.
		*repeat_flagp = 0;
		gap = 0;
	} else {
		// Calculate the time gap in microseconds.
		gap = time_elapsed_us;
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

	found=nullptr;
	found_code=0;
	have_code=0;
	codes=remote->codes;
	if(codes!=nullptr)
	{
		while(codes->name!=nullptr)
		{
			ir_code next_all;

			next_all = gen_ir_code(remote, remote->pre_data,
					       get_ir_code(codes, codes->current),
					       remote->post_data);
			if(match_ir_code(remote, next_all, all))
			{
				found_code=1;
				if(codes->next!=nullptr)
				{
					if(codes->current==nullptr)
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
					if(codes->current==nullptr)
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
				if(search == nullptr ||
					(codes->next != nullptr && codes->current == nullptr))
				{
					codes->current=nullptr;
				}
				else
				{
					int sequence_match = 0;
					
					while(search != codes->current->next)
					{
						struct ir_code_node *prev, *next;
						int flag = 1;
						
						prev = nullptr; /* means codes->code */
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
								if(!have_code)
								{
									found=codes;
								}
								break;
							}
						}
						search = search->next;
					}
					if(!sequence_match) codes->current = nullptr;
				}
			}
			codes++;
		}
	}

	if(found_code && found!=nullptr && has_toggle_mask(remote))
	{
		if(!(remote->toggle_mask_state%2))
		{
			remote->toggle_code=found;
		}
		else
		{
			if(found!=remote->toggle_code)
			{
				remote->toggle_code=nullptr;
				return(nullptr);
			}
			remote->toggle_code=nullptr;
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
	static struct ir_remote *last_decoded = nullptr;

	auto const current = std::chrono::steady_clock::now();

	if(remote==last_decoded &&
	   (found==remote->last_code || (found->next!=nullptr && found->current!=nullptr)) &&
	   repeat_flag &&
	   current - remote->last_send < 1s &&
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
		else if(found->current==nullptr)
		{
			remote->reps++;
		}
	}
	else
	{
		if(found->next!=nullptr && found->current==nullptr)
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
	last_decoded=remote;
	if(found->current==nullptr) remote->last_code=found;
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

WINLIRC_API bool decodeCommand(struct hardware const* phw, struct ir_remote *remotes, char *out, size_t out_size)
{
    auto& hw = *phw;
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
		//LOGPRINTF(1,"trying \"%s\" remote",remote->name);
		
		if(hw.decode_func(&hw,remote,&pre,&code,&post,&repeat_flag,
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
			   ncode->current!=nullptr)
			{
				decoding=nullptr;
				return(nullptr);
			}

			for(scan = decoding; scan != nullptr; scan = scan->next)
			{
				for( scan_ncode = scan->codes; scan_ncode->name != nullptr; scan_ncode++)
				{
					scan_ncode->current = nullptr;
				}
			}
			if(is_xmp(remote))
			{
				remote->last_code->current = remote->last_code->next;
			}
			
			len = write_message(out, out_size,
					    remote->name,
					    remote->last_code->name, "", code,
					    remote->reps-(ncode->next ? 1:0));
			decoding=nullptr;
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
			//LOGPRINTF(1,"failed \"%s\" remote",remote->name);
		}
		remote->toggle_mask_state=0;
		remote=remote->next;
	}
	decoding=nullptr;
	last_remote=nullptr;

	return false;
}