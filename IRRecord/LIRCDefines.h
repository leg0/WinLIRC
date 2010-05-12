#ifndef LIRCDEFINES_H
#define LIRCDEFINES_H

#include <Math.h>

//
// Constants
//
#define PULSE_BIT  0x01000000
#define PULSE_MASK 0x00FFFFFF

#define LONG_IR_CODE
#define PACKET_SIZE		(256)
#define RBUF_SIZE		(256)
#define WBUF_SIZE		(256)
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

#define LIRC_MODE2SEND(x) (x)
#define LIRC_SEND2MODE(x) (x)
#define LIRC_MODE2REC(x) ((x) << 16)
#define LIRC_REC2MODE(x) ((x) >> 16)

#define LIRC_MODE_RAW                  0x00000001
#define LIRC_MODE_PULSE                0x00000002
#define LIRC_MODE_MODE2                0x00000004
#define LIRC_MODE_CODE                 0x00000008
#define LIRC_MODE_LIRCCODE             0x00000010
#define LIRC_MODE_STRING               0x00000020

#define LIRC_CAN_SEND_RAW              LIRC_MODE2SEND(LIRC_MODE_RAW)
#define LIRC_CAN_SEND_PULSE            LIRC_MODE2SEND(LIRC_MODE_PULSE)
#define LIRC_CAN_SEND_MODE2            LIRC_MODE2SEND(LIRC_MODE_MODE2)
#define LIRC_CAN_SEND_CODE             LIRC_MODE2SEND(LIRC_MODE_CODE)
#define LIRC_CAN_SEND_LIRCCODE         LIRC_MODE2SEND(LIRC_MODE_LIRCCODE)
#define LIRC_CAN_SEND_STRING           LIRC_MODE2SEND(LIRC_MODE_STRING)

#define LIRC_CAN_SEND_MASK             0x0000003f

#define LIRC_CAN_SET_SEND_CARRIER      0x00000100
#define LIRC_CAN_SET_SEND_DUTY_CYCLE   0x00000200
#define LIRC_CAN_SET_TRANSMITTER_MASK  0x00000400

#define LIRC_CAN_REC_RAW               LIRC_MODE2REC(LIRC_MODE_RAW)
#define LIRC_CAN_REC_PULSE             LIRC_MODE2REC(LIRC_MODE_PULSE)
#define LIRC_CAN_REC_MODE2             LIRC_MODE2REC(LIRC_MODE_MODE2)
#define LIRC_CAN_REC_CODE              LIRC_MODE2REC(LIRC_MODE_CODE)
#define LIRC_CAN_REC_LIRCCODE          LIRC_MODE2REC(LIRC_MODE_LIRCCODE)
#define LIRC_CAN_REC_STRING            LIRC_MODE2REC(LIRC_MODE_STRING)

#define LIRC_CAN_REC_MASK              LIRC_MODE2REC(LIRC_CAN_SEND_MASK)

#define LIRC_CAN_SET_REC_CARRIER       (LIRC_CAN_SET_SEND_CARRIER << 16)
#define LIRC_CAN_SET_REC_DUTY_CYCLE    (LIRC_CAN_SET_SEND_DUTY_CYCLE << 16)

#define LIRC_CAN_SET_REC_DUTY_CYCLE_RANGE 0x40000000
#define LIRC_CAN_SET_REC_CARRIER_RANGE    0x80000000
#define LIRC_CAN_GET_REC_RESOLUTION       0x20000000

#define LIRC_CAN_SEND(x) ((x)&LIRC_CAN_SEND_MASK)
#define LIRC_CAN_REC(x) ((x)&LIRC_CAN_REC_MASK)

#define LIRC_CAN_NOTIFY_DECODE            0x01000000

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

//
// Functions
//
static inline ir_code get_ir_code(struct ir_ncode *ncode, struct ir_code_node *node)
{
	if(ncode->next && node != NULL) return node->code;
	return ncode->code;
}

static inline struct ir_code_node *get_next_ir_code_node(struct ir_ncode *ncode, struct ir_code_node *node)
{
	if(node == NULL) return ncode->next;
	return node->next;
}

static inline int bit_count(struct ir_remote *remote)
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

static inline int has_repeat(struct ir_remote *remote)
{
	if(remote->prepeat>0 && remote->srepeat>0) return(1);
	else return(0);
}

static inline void set_protocol(struct ir_remote *remote, int protocol)
{
	remote->flags&=~(IR_PROTOCOL_MASK);
	remote->flags|=protocol;
}

static inline int is_raw(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RAW_CODES) return(1);
	else return(0);
}

static inline int is_space_enc(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SPACE_ENC) return(1);
	else return(0);
}

static inline int is_space_first(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SPACE_FIRST) return(1);
	else return(0);
}

static inline int is_rc5(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RC5) return(1);
	else return(0);
}

static inline int is_rc6(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RC6 ||
	   remote->rc6_mask) return(1);
	else return(0);
}

static inline int is_biphase(struct ir_remote *remote)
{
	if(is_rc5(remote) || is_rc6(remote)) return(1);
	else return(0);
}

static inline int is_rcmm(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == RCMM) return(1);
	else return(0);
}

static inline int is_goldstar(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == GOLDSTAR) return(1);
	else return(0);
}

static inline int is_grundig(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == GRUNDIG) return(1);
	else return(0);
}

static inline int is_bo(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == BO) return(1);
	else return(0);
}

static inline int is_serial(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == SERIAL) return(1);
	else return(0);
}

static inline int is_xmp(struct ir_remote *remote)
{
	if((remote->flags&IR_PROTOCOL_MASK) == XMP) return(1);
	else return(0);
}

static inline int is_const(struct ir_remote *remote)
{
	if(remote->flags&CONST_LENGTH) return(1);
	else return(0);
}

static inline int has_repeat_gap(struct ir_remote *remote)
{
	if(remote->repeat_gap>0) return(1);
	else return(0);
}

static inline int has_pre(struct ir_remote *remote)
{
	if(remote->pre_data_bits>0) return(1);
	else return(0);
}

static inline int has_post(struct ir_remote *remote)
{
	if(remote->post_data_bits>0) return(1);
	else return(0);
}

static inline int has_header(struct ir_remote *remote)
{
	if(remote->phead>0 && remote->shead>0) return(1);
	else return(0);
}

static inline int has_foot(struct ir_remote *remote)
{
	if(remote->pfoot>0 && remote->sfoot>0) return(1);
	else return(0);
}

static inline int has_toggle_bit_mask(struct ir_remote *remote)
{
	if(remote->toggle_bit_mask>0) return(1);
	else return(0);
}

static inline int has_ignore_mask(struct ir_remote *remote)
{
	if(remote->ignore_mask>0) return(1);
	else return(0);
}

static inline int has_toggle_mask(struct ir_remote *remote)
{
	if(remote->toggle_mask>0) return(1);
	else return(0);
}

static inline lirc_t min_gap(struct ir_remote *remote)
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

static inline lirc_t max_gap(struct ir_remote *remote)
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

static inline unsigned long time_elapsed(struct mytimeval *last,
					 struct mytimeval *current)
{
	__int64 secs,diff;
	
	secs=current->tv_sec-last->tv_sec;
	
	diff=1000000*secs+current->tv_usec-last->tv_usec;
	
	return(unsigned long(diff));
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

static inline ir_code gen_ir_code(struct ir_remote *remote, ir_code pre, ir_code code, ir_code post)
{
	ir_code all;
	
	all = (pre&gen_mask(remote->pre_data_bits));
	all <<= remote->bits;
	all |= is_raw(remote) ? code:(code&gen_mask(remote->bits));
	all <<= remote->post_data_bits;
	all |= post&gen_mask(remote->post_data_bits);

	return all;
}

static int match_ir_code(struct ir_remote *remote, ir_code a, ir_code b)
{
	return ((remote->ignore_mask|a) == (remote->ignore_mask|b) || (remote->ignore_mask|a) == (remote->ignore_mask|(b^remote->toggle_bit_mask)));
}

static inline int expect(struct ir_remote *remote,lirc_t delta,lirc_t exdelta)
{
	int aeps = remote->aeps;
	
	if(abs(exdelta-delta)<=exdelta*remote->eps/100 ||
	   abs(exdelta-delta)<=aeps)
		return 1;
	return 0;
}

static inline int expect_at_least(struct ir_remote *remote,
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

static inline int expect_at_most(struct ir_remote *remote,
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


#endif
