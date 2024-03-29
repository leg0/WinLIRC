#include "constants.h"
#include "ir_remote.h"
#include <winlirc/winlirc_api.h>
#include <algorithm>
#include <string.h>

#define LIRCD_EXACT_GAP_THRESHOLD 10000

WINLIRC_API int winlirc_get_send_buffer_length(sbuf const* send_buffer)
{
    return send_buffer->wptr;
}

WINLIRC_API lirc_t const* winlirc_get_send_buffer_data(sbuf const* send_buffer)
{
    return send_buffer->data;
}

WINLIRC_API void winlirc_init_send_buffer(sbuf* send_buffer)
{
	memset(send_buffer, 0, sizeof(*send_buffer));
}

static void clear_send_buffer(sbuf& send_buffer)
{
	send_buffer.wptr=0;
	send_buffer.too_long=0;
	send_buffer.is_biphase=0;
	send_buffer.pendingp=0;
	send_buffer.pendings=0;
	send_buffer.sum=0;
}

static void add_send_buffer(sbuf& send_buffer, lirc_t data)
{
	if(send_buffer.wptr<WBUF_SIZE)
	{
		send_buffer.sum+=data;
		send_buffer._data[send_buffer.wptr]=data;
		send_buffer.wptr++;
	}
	else
	{
		send_buffer.too_long=1;
	}
}

static void send_pulse(sbuf& send_buffer, lirc_t data)
{
	if(send_buffer.pendingp>0)
	{
		send_buffer.pendingp+=data;
	}
	else
	{
		if(send_buffer.pendings>0)
		{
			add_send_buffer(send_buffer, send_buffer.pendings);
			send_buffer.pendings=0;
		}
		send_buffer.pendingp=data;
	}
}

static void send_space(sbuf& send_buffer, lirc_t data)
{
	if(send_buffer.wptr==0 && send_buffer.pendingp==0)
	{
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
			add_send_buffer(send_buffer, send_buffer.pendingp);
			send_buffer.pendingp=0;
		}
		send_buffer.pendings=data;
	}
}

static int bad_send_buffer(sbuf const& send_buffer)
{
	if(send_buffer.too_long!=0) return(1);
	if(send_buffer.wptr==WBUF_SIZE && send_buffer.pendingp>0)
	{
		return(1);
	}
	return(0);
}

static int check_send_buffer(sbuf const& send_buffer)
{
	int i;

	if (send_buffer.wptr == 0) 
	{
		return(0);
	}
	for (i = 0; i < send_buffer.wptr; i++)
	{
		if(send_buffer.data[i] == 0)
		{
			return 0;
		}
	}
	
	return 1;
}

static void flush_send_buffer(sbuf& send_buffer)
{
	if(send_buffer.pendingp>0)
	{
		add_send_buffer(send_buffer, send_buffer.pendingp);
		send_buffer.pendingp=0;
	}
	if(send_buffer.pendings>0)
	{
		add_send_buffer(send_buffer, send_buffer.pendings);
		send_buffer.pendings=0;
	}
}

static void sync_send_buffer(sbuf& send_buffer)
{
	if(send_buffer.pendingp>0)
	{
		add_send_buffer(send_buffer, send_buffer.pendingp);
		send_buffer.pendingp=0;
	}
	if(send_buffer.wptr>0 && send_buffer.wptr%2==0) send_buffer.wptr--;
}

static void send_header(sbuf& send_buffer, ir_remote *remote)
{
	if(has_header(remote))
	{
		send_pulse(send_buffer, remote->phead);
		send_space(send_buffer, remote->shead);
	}
}

static void send_foot(sbuf& send_buffer, ir_remote *remote)
{
	if(has_foot(remote))
	{
		send_space(send_buffer, remote->sfoot);
		send_pulse(send_buffer, remote->pfoot);
	}
}

static void send_lead(sbuf& send_buffer, ir_remote *remote)
{
	if(remote->plead!=0)
	{
		send_pulse(send_buffer, remote->plead);
	}
}

static void send_trail(sbuf& send_buffer, ir_remote *remote)
{
	if(remote->ptrail!=0)
	{
		send_pulse(send_buffer, remote->ptrail);
	}
}

static void send_data(sbuf& send_buffer, ir_remote *remote,ir_code data,int bits,int done)
{
	auto const all_bits = bit_count(remote);
	auto const toggle_bit_mask_bits = bits_set(remote->toggle_bit_mask);
	ir_code mask;
	
	data=reverse(data,bits);
	if(is_rcmm(remote))
	{
		mask=(ir_code)1<<(all_bits-1-done);
		if(bits%2 || done%2)
		{
			return;
		}
		for(int i=0;i<bits;i+=2,mask>>=2)
		{
			switch(data&3)
			{
			case 0:
				send_pulse(send_buffer, remote->pzero);
				send_space(send_buffer, remote->szero);
				break;
			/* 2 and 1 swapped due to reverse() */
			case 2:
				send_pulse(send_buffer, remote->pone);
				send_space(send_buffer, remote->sone);
				break;
			case 1:
				send_pulse(send_buffer, remote->ptwo);
				send_space(send_buffer, remote->stwo);
				break;
			case 3:
				send_pulse(send_buffer, remote->pthree);
				send_space(send_buffer, remote->sthree);
				break;
			}
			data=data>>2;
		}
		return;
	}
	else if(is_xmp(remote))
	{
		if(bits%4 || done%4)
		{
			return;
		}
		for(int i = 0; i < bits; i += 4)
		{
			ir_code nibble = reverse(data & 0xf, 4);
			send_pulse(send_buffer, remote->pzero);
			send_space(send_buffer, (unsigned long)(remote->szero + nibble*remote->sone));
			data >>= 4;
		}
		return;
	}

	mask=((ir_code) 1)<<(all_bits-1-done);
	for(int i=0;i<bits;i++,mask>>=1)
	{
		if(has_toggle_bit_mask(remote) && mask&remote->toggle_bit_mask)
		{
			if(toggle_bit_mask_bits == 1)
			{
				/* backwards compatibility */
				data &= ~((ir_code) 1);
				if(remote->toggle_bit_mask_state&mask)
				{
					data |= (ir_code) 1;
				}
			}
			else
			{
				if(remote->toggle_bit_mask_state&mask)
				{
					data ^= (ir_code) 1;
				}
			}
		}
		if(has_toggle_mask(remote) &&
		   mask&remote->toggle_mask &&
		   remote->toggle_mask_state%2)
		{
			data ^= 1;
		}
		if(data&1)
		{
			if(is_biphase(remote))
			{
				
				if(mask&remote->rc6_mask)
				{
					send_space(send_buffer, 2*remote->sone);
					send_pulse(send_buffer, 2*remote->pone);
				}
				else
				{
					send_space(send_buffer, remote->sone);
					send_pulse(send_buffer, remote->pone);
				}
			}
			else if(is_space_first(remote))
			{
				send_space(send_buffer, remote->sone);
				send_pulse(send_buffer, remote->pone);
			}
			else
			{
				send_pulse(send_buffer, remote->pone);
				send_space(send_buffer, remote->sone);
			}
		}
		else
		{
			if(mask&remote->rc6_mask)
			{
				send_pulse(send_buffer, 2*remote->pzero);
				send_space(send_buffer, 2*remote->szero);
			}
			else if(is_space_first(remote))
			{
				send_space(send_buffer, remote->szero);
				send_pulse(send_buffer, remote->pzero);
			}
			else
			{
				send_pulse(send_buffer, remote->pzero);
				send_space(send_buffer, remote->szero);
			}
		}
		data=data>>1;
	}
}

static void send_pre(sbuf& send_buffer, ir_remote *remote)
{
	if(has_pre(remote))
	{
		send_data(send_buffer, remote,remote->pre_data,remote->pre_data_bits,0);
		if(remote->pre_p>0 && remote->pre_s>0)
		{
			send_pulse(send_buffer, remote->pre_p);
			send_space(send_buffer, remote->pre_s);
		}
	}
}

static void send_post(sbuf& send_buffer, ir_remote *remote)
{
	if(has_post(remote))
	{
		if(remote->post_p>0 && remote->post_s>0)
		{
			send_pulse(send_buffer, remote->post_p);
			send_space(send_buffer, remote->post_s);
		}
		send_data(send_buffer, remote,remote->post_data,remote->post_data_bits,
			  remote->pre_data_bits+remote->bits);
	}
}

static void send_repeat(sbuf& send_buffer, ir_remote *remote)
{
	send_lead(send_buffer, remote);
	send_pulse(send_buffer, remote->prepeat);
	send_space(send_buffer, remote->srepeat);
	send_trail(send_buffer, remote);
}

static void send_code(sbuf& send_buffer, ir_remote *remote,ir_code code, int repeat)
{
	if(!repeat || !(remote->flags&NO_HEAD_REP))
		send_header(send_buffer, remote);
	send_lead(send_buffer, remote);
	send_pre(send_buffer, remote);
	send_data(send_buffer, remote, code, remote->bits, remote->pre_data_bits);
	send_post(send_buffer, remote);
	send_trail(send_buffer, remote);
	if (!repeat || !(remote->flags & NO_FOOT_REP))
		send_foot(send_buffer, remote);
	
	if(!repeat &&
	   remote->flags&NO_HEAD_REP &&
	   remote->flags&CONST_LENGTH)
	{
		send_buffer.sum-=remote->phead+remote->shead;
	}
}

static void send_signals(sbuf& send_buffer, std::vector<lirc_t> const& signals)
{
	for (auto& signal : signals)
	{
		add_send_buffer(send_buffer, signal);
	}
}

WINLIRC_API int winlirc_init_send(sbuf* psend_buffer, ir_remote *remote, ir_ncode *code, int repeats)
{
	int repeat=0;
	auto& send_buffer = *psend_buffer;
	if(is_grundig(remote) ||  is_goldstar(remote) || is_serial(remote) || is_bo(remote))
	{
		return(0);
	}
	clear_send_buffer(send_buffer);
	if(is_biphase(remote))
	{
		send_buffer.is_biphase=1;
	}

	remote->repeat_countdown = std::max(remote->min_repeat,repeats);
	
 init_send_loop:
	if(repeat && has_repeat(remote))
	{
		if(remote->flags&REPEAT_HEADER && has_header(remote))
		{
			send_header(send_buffer, remote);
		}
		send_repeat(send_buffer, remote);
	}
	else
	{
		if(!is_raw(remote))
		{	
			ir_code next_code;
			
			if(code->transmit_state == nullptr)
			{
				next_code = code->code;
			}
			else
			{
				next_code = code->transmit_state->code;
			}

			send_code(send_buffer, remote, next_code, repeat);

			if(has_toggle_mask(remote))
			{
				remote->toggle_mask_state++;
				if(remote->toggle_mask_state==4)
				{
					remote->toggle_mask_state=2;
				}
			}
		}
		else
		{
			if(code->signals.empty())
			{
				return 0;
			}
			
			send_signals(send_buffer, code->signals);
		}
	}
	sync_send_buffer(send_buffer);
	if(bad_send_buffer(send_buffer))
	{
		return(0);
	}
	if(has_repeat_gap(remote) && repeat && has_repeat(remote))
	{
		remote->min_remaining_gap=remote->repeat_gap;
		remote->max_remaining_gap=remote->repeat_gap;
	}
	else if(is_const(remote))
	{
		if(min_gap(remote)>send_buffer.sum)
		{
			remote->min_remaining_gap=min_gap(remote)-send_buffer.sum;
			remote->max_remaining_gap=max_gap(remote)-send_buffer.sum;
		}
		else
		{
			remote->min_remaining_gap=min_gap(remote);
			remote->max_remaining_gap=max_gap(remote);
			return(0);
		}
	}
	else
	{
		remote->min_remaining_gap=min_gap(remote);
		remote->max_remaining_gap=max_gap(remote);
	}

	// only used for two part codes
	if(code->next != nullptr)
	{
		if(code->transmit_state == nullptr)
		{
			code->transmit_state = code->next.get();
		}
		else
		{
			code->transmit_state = code->transmit_state->next.get();
			if(is_xmp(remote) && code->transmit_state == nullptr)
			{
				code->transmit_state = code->next.get();
			}
		}
	}

	send_space(send_buffer, remote->min_remaining_gap);
	flush_send_buffer(send_buffer);

	if(remote->repeat_countdown>0 || code->transmit_state != nullptr )
	{
		if(code->next == nullptr || code->transmit_state == nullptr)
		{
			remote->repeat_countdown--;
			repeat = 1;
		}
		
		send_buffer.sum	= 0;
		
		goto init_send_loop;
	}

	send_buffer.data=send_buffer._data;

	if(!check_send_buffer(send_buffer))
	{
		return 0;
	}
	
	return 1;
}