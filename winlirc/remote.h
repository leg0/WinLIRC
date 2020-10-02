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
 * Copyright (C) 1996,97 Ralph Metzler <rjkm@thp.uni-koeln.de>
 * Copyright (C) 1998 Christoph Bartelmus <columbus@hit.handshake.de>
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Copyright (C) 2002 Scott Baily <baily@uiuc.edu>
 */

#pragma once

#include "globals.h"

#include <sys/types.h>
#include <string.h>
#include <math.h>

#define strcasecmp _stricmp

struct ir_ncode;
struct ir_code_node;
struct ir_remote;

//
// Functions
//
static inline ir_code get_ir_code(ir_ncode* ncode, ir_code_node* node)
{
	if(ncode->next && node != nullptr) return node->code;
	return ncode->code;
}

static inline ir_code_node* get_next_ir_code_node(ir_ncode* ncode, ir_code_node *node)
{
	if(node == nullptr) return ncode->next;
	return node->next;
}

static inline int bit_count(ir_remote*remote)
{
	return remote->pre_data_bits +
		remote->bits +
		remote->post_data_bits;
}

static inline int bits_set(ir_code data)
{
	int ret = 0;
	while(data)
	{
		if(data&1) ret++;
		data >>= 1;
	}
	return ret;
}

static inline ir_code reverse(ir_code data,int bits)
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

static inline int is_pulse(lirc_t data)
{
	return(data&PULSE_BIT ? 1:0);
}

static inline int is_space(lirc_t data)
{
	return(!is_pulse(data));
}

static inline int has_repeat(ir_remote*remote)
{
	if(remote->prepeat>0 && remote->srepeat>0) return(1);
	else return(0);
}

static inline void set_protocol(ir_remote*remote, int protocol)
{
	remote->flags&=~(IR_PROTOCOL_MASK);
	remote->flags|=protocol;
}

static inline int is_raw(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RAW_CODES) return(1);
	else return(0);
}

static inline int is_space_enc(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SPACE_ENC) return(1);
	else return(0);
}

static inline int is_space_first(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SPACE_FIRST) return(1);
	else return(0);
}

static inline int is_rc5(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RC5) return(1);
	else return(0);
}

static inline int is_rc6(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RC6 ||
	   remote->rc6_mask) return(1);
	else return(0);
}

static inline int is_biphase(ir_remote*remote)
{
	if(is_rc5(remote) || is_rc6(remote)) return(1);
	else return(0);
}

static inline int is_rcmm(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RCMM) return(1);
	else return(0);
}

static inline int is_goldstar(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == GOLDSTAR) return(1);
	else return(0);
}

static inline int is_grundig(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == GRUNDIG) return(1);
	else return(0);
}

static inline int is_bo(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == BO) return(1);
	else return(0);
}

static inline int is_serial(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SERIAL) return(1);
	else return(0);
}

static inline int is_xmp(ir_remote*remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == XMP) return(1);
	else return(0);
}

static inline int is_const(ir_remote*remote)
{
	if(remote->flags&CONST_LENGTH) return(1);
	else return(0);
}

static inline int has_repeat_gap(ir_remote*remote)
{
	if(remote->repeat_gap>0) return(1);
	else return(0);
}

static inline int has_pre(ir_remote*remote)
{
	if(remote->pre_data_bits>0) return(1);
	else return(0);
}

static inline int has_post(ir_remote*remote)
{
	if(remote->post_data_bits>0) return(1);
	else return(0);
}

static inline int has_header(ir_remote*remote)
{
	if(remote->phead>0 && remote->shead>0) return(1);
	else return(0);
}

static inline int has_foot(ir_remote*remote)
{
	if(remote->pfoot>0 && remote->sfoot>0) return(1);
	else return(0);
}

static inline int has_toggle_bit_mask(ir_remote*remote)
{
	if(remote->toggle_bit_mask>0) return(1);
	else return(0);
}

static inline int has_ignore_mask(ir_remote*remote)
{
	if(remote->ignore_mask>0) return(1);
	else return(0);
}

static inline int has_toggle_mask(ir_remote*remote)
{
	if(remote->toggle_mask>0) return(1);
	else return(0);
}

static inline lirc_t min_gap(ir_remote*remote)
{
	if(remote->gap2 != 0 && remote->gap2 < remote->gap)
	{
		return remote->gap2;
	}
	else
	{
		return remote->gap;
	}
}

static inline lirc_t max_gap(ir_remote*remote)
{
	if(remote->gap2 > remote->gap)
	{
		return remote->gap2;
	}
	else
	{
		return remote->gap;
	}
}

static inline ir_code gen_mask(int bits)
{
	int i;
	ir_code mask;

	mask=0;
	for(i=0;i<bits;i++)
	{
		mask<<=1;
		mask|=1;
	}
	return(mask);
}

static inline ir_code gen_ir_code(ir_remote*remote, ir_code pre, ir_code code, ir_code post)
{
	ir_code all;
	
	all = (pre&gen_mask(remote->pre_data_bits));
	all <<= remote->bits;
	all |= is_raw(remote) ? code:(code&gen_mask(remote->bits));
	all <<= remote->post_data_bits;
	all |= post&gen_mask(remote->post_data_bits);

	return all;
}

static int match_ir_code(ir_remote*remote, ir_code a, ir_code b)
{
	return ((remote->ignore_mask|a) == (remote->ignore_mask|b) || (remote->ignore_mask|a) == (remote->ignore_mask|(b^remote->toggle_bit_mask)));
}

static inline int expect(ir_remote*remote,lirc_t delta,lirc_t exdelta)
{
	int aeps = remote->aeps;
	
	if(abs(exdelta-delta)<=exdelta*remote->eps/100 ||
	   abs(exdelta-delta)<=aeps)
		return 1;
	return 0;
}

static inline int expect_at_least(ir_remote*remote,
				  lirc_t delta, lirc_t exdelta)
{
	int aeps = remote->aeps;
	
	if(delta+exdelta*remote->eps/100>=exdelta ||
	   delta+aeps>=exdelta)
	{
		return 1;
	}
	return 0;
}

static inline int expect_at_most(ir_remote*remote,
				 lirc_t delta, lirc_t exdelta)
{
	int aeps = remote->aeps;
	
	if(delta<=exdelta+exdelta*remote->eps/100 ||
	   delta<=exdelta+aeps)
	{
		return 1;
	}
	return 0;
}

static inline ir_remote* get_remote_by_name(ir_remote* remotes, const char *name)
{
	while (remotes!=nullptr&& _stricmp(name,remotes->name)) {
		remotes = remotes->next;
	}

	return remotes;
}

static inline ir_ncode* get_code_by_name(ir_ncode*codes, const char *name)
{
	while (codes->name!=nullptr&& _stricmp(name,codes->name)) {
		codes++;	// linked list would have been easier .. 
	}

	return codes;
}
