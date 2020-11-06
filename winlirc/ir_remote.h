#pragma once

#include <winlirc/winlirc_api.h>

#include <chrono>

template <typename T>
struct void_array
{
	T* ptr;
	size_t nr_items;
	size_t chunk_size;

	T& operator[](size_t idx) { return ptr[idx]; }
	T const& operator[](size_t idx) const { return ptr[idx]; }
};

template <typename T> void init_void_array(void_array<T>& ar, size_t chunk_size);
template <typename T> int add_void_array(void_array<T>& ar, T const& data);
template <typename T> T* get_void_array(void_array<T>& ar);
template <typename T> void free_void_array(void_array<T>& ar)
{
	if (ar.ptr != nullptr)
		free(ar.ptr);
	ar.ptr = nullptr;
	ar.nr_items = 0;
}

struct ir_code_node
{
	ir_code code;
	ir_code_node *next;
};

struct ir_ncode
{
	char* name;
	ir_code code;
	size_t length() const { return signals.nr_items; }
	void_array<lirc_t> signals;
	ir_code_node *next;
	ir_code_node *current;
	ir_code_node *transmit_state;
};

struct ir_remote
{
	char* name;                 /* name of remote control */
	ir_ncode* codes;
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
	ir_ncode* last_code; /* code received or sent last */
	ir_ncode* toggle_code;/* toggle code received or sent last */
	int reps;
	std::chrono::steady_clock::time_point last_send;   /* time last_code was received or sent */
	lirc_t min_remaining_gap;   /* remember gap for CONST_LENGTH remotes */
	lirc_t max_remaining_gap;   /* gap range */
	ir_remote* next;
};
