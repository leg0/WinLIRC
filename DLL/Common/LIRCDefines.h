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

#pragma once
#include <Math.h>
#include <winlirc/winlirc_api.h>
#include <chrono>

//
// Constants
//
#define PULSE_BIT  0x01000000
#define PULSE_MASK 0x00FFFFFF

#define LONG_IR_CODE
#define PACKET_SIZE		(256)
#define REC_SYNC		8

#define IR_PROTOCOL_MASK 0x07ff

#define RAW_CODES       0x0001		/* for internal use only */
#define RC5             0x0002		/* IR data follows RC5 protocol */
#define SHIFT_ENC		RC5			/* IR data is shift encoded (name obsolete) */
#define RC6             0x0004		/* IR data follows RC6 protocol */
#define RCMM            0x0008		/* IR data follows RC-MM protocol */
#define SPACE_ENC		0x0010		/* IR data is space encoded */
#define SPACE_FIRST     0x0020		/* bits are encoded as space+pulse */
#define GOLDSTAR        0x0040		/* encoding found on Goldstar remote */
#define GRUNDIG         0x0080		/* encoding found on Grundig remote */
#define BO              0x0100		/* encoding found on Bang & Olufsen remote */
#define SERIAL          0x0200		/* serial protocol */
#define XMP             0x0400		/* XMP protocol */

/* additinal flags: can be orred together with protocol flag */
#define REVERSE			0x0800
#define NO_HEAD_REP		0x1000		/* no header for key repeats */
#define NO_FOOT_REP		0x2000		/* no foot for key repeats */
#define CONST_LENGTH    0x4000		/* signal length+gap is always constant */
#define REPEAT_HEADER   0x8000		/* header is also sent before repeat code */

#define COMPAT_REVERSE  0x00010000	/* compatibility mode for REVERSE flag */

#define REPEAT_MAX_DEFAULT 600

#define DEFAULT_FREQ 38000

#define IR_PARITY_NONE 0
#define IR_PARITY_EVEN 1
#define IR_PARITY_ODD  2


//
//Structure definitions
//

struct ir_code_node
{
	ir_code code;
	struct ir_code_node *next;
};

struct ir_ncode {
	char *name;
	ir_code code;
	int length;
    lirc_t *signals;
	struct ir_code_node *next;
	struct ir_code_node *current;
	struct ir_code_node *transmit_state;
};

struct ir_remote
{
	char *name;                 /* name of remote control */
	struct ir_ncode *codes;
	int bits;                   /* bits (length of code) */
	int flags;                  /* flags */
	int eps;                    /* eps (_relative_ tolerance) */
	int aeps;                   /* detecing _very short_ pulses is
								   difficult with relative tolerance
								   for some remotes,
								   this is an _absolute_ tolerance
								   to solve this problem
								   usually you can say 0 here */
	
	/* pulse and space lengths of: */
	
	lirc_t phead,shead;         /* header */
	lirc_t pthree,sthree;       /* 3 (only used for RC-MM) */
	lirc_t ptwo,stwo;           /* 2 (only used for RC-MM) */
	lirc_t pone,sone;           /* 1 */
	lirc_t pzero,szero;         /* 0 */
	lirc_t plead;				/* leading pulse */
	lirc_t ptrail;              /* trailing pulse */
	lirc_t pfoot,sfoot;         /* foot */
	lirc_t prepeat,srepeat;	    /* indicate repeating */

	int pre_data_bits;          /* length of pre_data */
	ir_code pre_data;           /* data which the remote sends before actual keycode */
	int post_data_bits;         /* length of post_data */
	ir_code post_data;          /* data which the remote sends after actual keycode */
	lirc_t pre_p,pre_s;         /* signal between pre_data and keycode */
	lirc_t post_p, post_s;      /* signal between keycode and post_code */

	lirc_t gap;                 /* time between signals in usecs */
	lirc_t gap2;                /* time between signals in usecs */
	lirc_t repeat_gap;          /* time between two repeat codes if different from gap */
	int toggle_bit;             /* obsolete */
	ir_code toggle_bit_mask;    /* previously only one bit called toggle_bit */
	int min_repeat;             /* code is repeated at least x times code sent once -> min_repeat=0 */
	unsigned int min_code_repeat;/*meaningful only if remote sends a repeat code: in this case this value indicates how often the real code is repeated before the repeat code is being sent */
	unsigned int freq;          /* modulation frequency */
	unsigned int duty_cycle;    /* 0<duty cycle<=100 */
	ir_code toggle_mask;        /* Sharp (?) error detection scheme */
	ir_code rc6_mask;           /* RC-6 doubles signal length of some bits */
	
	/* serial protocols */
	unsigned int baud;          /* can be overridden by [p|s]zero, [p|s]one */
	unsigned int bits_in_byte;  /* default: 8 */
	unsigned int parity;        /* currently unsupported */
	unsigned int stop_bits;     /* mapping: 1->2 1.5->3 2->4 */
	
	ir_code ignore_mask;        /* mask defines which bits can be ignored when matching a code */
	/* end of user editable values */
	
	ir_code toggle_bit_mask_state;
	int toggle_mask_state;
	int repeat_countdown;
	struct ir_ncode *last_code; /* code received or sent last */
	struct ir_ncode *toggle_code;/* toggle code received or sent last */
	int reps;
	std::chrono::steady_clock::time_point last_send;   /* time last_code was received or sent */
	lirc_t min_remaining_gap;   /* remember gap for CONST_LENGTH remotes */
	lirc_t max_remaining_gap;   /* gap range */
	ir_remote*next;
};

//
// Functions
//
static inline ir_code get_ir_code(struct ir_ncode *ncode, struct ir_code_node *node)
{
	if(ncode->next && node != nullptr) return node->code;
	return ncode->code;
}

static inline struct ir_code_node *get_next_ir_code_node(struct ir_ncode *ncode, struct ir_code_node *node)
{
	if(node == nullptr) return ncode->next;
	return node->next;
}

static inline int bit_count(ir_remote const* remote)
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

static inline int has_repeat(ir_remote const* remote)
{
	if(remote->prepeat>0 && remote->srepeat>0) return(1);
	else return(0);
}

static inline void set_protocol(ir_remote* remote, int protocol)
{
	remote->flags&=~(IR_PROTOCOL_MASK);
	remote->flags|=protocol;
}

static inline int is_raw(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RAW_CODES) return(1);
	else return(0);
}

static inline int is_space_enc(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SPACE_ENC) return(1);
	else return(0);
}

static inline int is_space_first(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SPACE_FIRST) return(1);
	else return(0);
}

static inline int is_rc5(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RC5) return(1);
	else return(0);
}

static inline int is_rc6(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RC6 ||
	   remote->rc6_mask) return(1);
	else return(0);
}

static inline int is_biphase(ir_remote const* remote)
{
	if(is_rc5(remote) || is_rc6(remote)) return(1);
	else return(0);
}

static inline int is_rcmm(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RCMM) return(1);
	else return(0);
}

static inline int is_goldstar(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == GOLDSTAR) return(1);
	else return(0);
}

static inline int is_grundig(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == GRUNDIG) return(1);
	else return(0);
}

static inline int is_bo(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == BO) return(1);
	else return(0);
}

static inline int is_serial(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SERIAL) return(1);
	else return(0);
}

static inline int is_xmp(ir_remote const* remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == XMP) return(1);
	else return(0);
}

static inline int is_const(ir_remote const* remote)
{
	if(remote->flags&CONST_LENGTH) return(1);
	else return(0);
}

static inline int has_repeat_gap(ir_remote const* remote)
{
	if(remote->repeat_gap>0) return(1);
	else return(0);
}

static inline int has_pre(ir_remote const* remote)
{
	if(remote->pre_data_bits>0) return(1);
	else return(0);
}

static inline int has_post(ir_remote const* remote)
{
	if(remote->post_data_bits>0) return(1);
	else return(0);
}

static inline int has_header(ir_remote const* remote)
{
	if(remote->phead>0 && remote->shead>0) return(1);
	else return(0);
}

static inline int has_foot(ir_remote const* remote)
{
	if(remote->pfoot>0 && remote->sfoot>0) return(1);
	else return(0);
}

static inline int has_toggle_bit_mask(ir_remote const* remote)
{
	if(remote->toggle_bit_mask>0) return(1);
	else return(0);
}

static inline int has_ignore_mask(ir_remote const* remote)
{
	if(remote->ignore_mask>0) return(1);
	else return(0);
}

static inline int has_toggle_mask(ir_remote const* remote)
{
	if(remote->toggle_mask>0) return(1);
	else return(0);
}

static inline lirc_t min_gap(ir_remote const* remote)
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

static inline lirc_t max_gap(ir_remote const* remote)
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

static inline ir_code gen_ir_code(ir_remote const* remote, ir_code pre, ir_code code, ir_code post)
{
	ir_code all;
	
	all = (pre&gen_mask(remote->pre_data_bits));
	all <<= remote->bits;
	all |= is_raw(remote) ? code:(code&gen_mask(remote->bits));
	all <<= remote->post_data_bits;
	all |= post&gen_mask(remote->post_data_bits);

	return all;
}

static int match_ir_code(ir_remote const* remote, ir_code a, ir_code b)
{
	return ((remote->ignore_mask|a) == (remote->ignore_mask|b) || (remote->ignore_mask|a) == (remote->ignore_mask|(b^remote->toggle_bit_mask)));
}

static inline int expect(ir_remote const* remote,lirc_t delta,lirc_t exdelta)
{
	int aeps = remote->aeps;
	
	if(abs(exdelta-delta)<=exdelta*remote->eps/100 ||
	   abs(exdelta-delta)<=aeps)
		return 1;
	return 0;
}

static inline int expect_at_least(ir_remote const* remote,
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

static inline int expect_at_most(ir_remote const* remote,
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
