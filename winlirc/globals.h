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
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Modifications based on LIRC 0.6.1 Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>

/* Debugging stuff */
extern bool debug;
extern FILE *debugfile;
extern char *__file;
extern int __line;
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG if(debug) __file=__FILE__,__line=__LINE__,winlirc_debug
extern void winlirc_debug(char *format, ...);
/* End of Debugging stuff */

/* Constants */
#define CBUF_LEN 1024
#include "version.h"
#define BUTTON 80
#define MAX_SIGNALS 200
#define WBUF_SIZE (256)
#define RBUF_SIZE (256)
#define PACKET_SIZE (256)
#define PULSE_BIT 0x1000000
#define REC_SYNC 8
#define WM_TRAY (WM_USER+34)
#define MAX_CLIENTS 8
#define IR_PORT 8765
#define LISTENQ 4
#define LONG_IR_CODE


//new stuff !!!!!!!

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

#define LIRC_MODE_RAW		0x00000001
#define LIRC_MODE_PULSE     0x00000002
#define LIRC_MODE_MODE2     0x00000004
#define LIRC_MODE_CODE      0x00000008
#define LIRC_MODE_LIRCCODE  0x00000010
#define LIRC_MODE_STRING    0x00000020


//
// Typedefs
//
#ifdef LONG_IR_CODE
typedef unsigned __int64 ir_code;
#else
typedef unsigned long ir_code;
#endif

typedef int lirc_t;

//
//Structure definitions
//

struct rbuf
{
	lirc_t data[RBUF_SIZE];
	ir_code decoded;
	int rptr;
	int wptr;
	int too_long;
	int is_biphase;
	lirc_t pendingp;
	lirc_t pendings;
	lirc_t sum;
};

struct sbuf
{
	lirc_t *data;
	lirc_t _data[WBUF_SIZE];
	int wptr;
	int too_long;
	int is_biphase;
	lirc_t pendingp;
	lirc_t pendings;
	lirc_t sum;
};

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

struct mytimeval {
	__int64 tv_sec;
	__int64 tv_usec;
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
	struct mytimeval last_send;   /* time last_code was received or sent */
	lirc_t min_remaining_gap;   /* remember gap for CONST_LENGTH remotes */
	lirc_t max_remaining_gap;   /* gap range */
	struct ir_remote *next;
};



/* externs */
extern struct ir_remote remote;
extern struct ir_ncode code;
extern struct ir_remote *global_remotes;
extern class CIRDriver *ir_driver;
extern class Clearndlg *learn_dialog;
extern int line;
extern int parse_error;
extern int lirc;


/* Change this stuff */
extern class CCriticalSection CS_global_remotes;
extern class CWinThread *ServerThreadHandle;
extern class CEvent ServerThreadEvent;

extern class CIRConfig config;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent);
void KillThread2(CWinThread **ThreadHandle, HANDLE ThreadEvent);

#endif