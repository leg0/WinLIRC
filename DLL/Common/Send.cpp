#include "Send.h"
#include <Windows.h>
#include "LIRCDefines.h"

#define LIRCD_EXACT_GAP_THRESHOLD 10000

static struct sbuf send_buffer;

WINLIRC_API int get_send_buffer_length(void)
{
    return send_buffer.wptr;
}

WINLIRC_API lirc_t const* get_send_buffer_data(void)
{
    return send_buffer.data;
}

WINLIRC_API void init_send_buffer(void)
{
	memset(&send_buffer,0,sizeof(send_buffer));
}

void clear_send_buffer(void)
{
	send_buffer.wptr=0;
	send_buffer.too_long=0;
	send_buffer.is_biphase=0;
	send_buffer.pendingp=0;
	send_buffer.pendings=0;
	send_buffer.sum=0;
}

void add_send_buffer(lirc_t data)
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

void send_pulse(lirc_t data)
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

void send_space(lirc_t data)
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
			add_send_buffer(send_buffer.pendingp);
			send_buffer.pendingp=0;
		}
		send_buffer.pendings=data;
	}
}

int bad_send_buffer(void)
{
	if(send_buffer.too_long!=0) return(1);
	if(send_buffer.wptr==WBUF_SIZE && send_buffer.pendingp>0)
	{
		return(1);
	}
	return(0);
}

int check_send_buffer(void)
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
			if(i%2)
			{

			}
			else
			{

			}
			return 0;
		}
	}
	
	return 1;
}

void flush_send_buffer(void)
{
	if(send_buffer.pendingp>0)
	{
		add_send_buffer(send_buffer.pendingp);
		send_buffer.pendingp=0;
	}
	if(send_buffer.pendings>0)
	{
		add_send_buffer(send_buffer.pendings);
		send_buffer.pendings=0;
	}
}

void sync_send_buffer(void)
{
	if(send_buffer.pendingp>0)
	{
		add_send_buffer(send_buffer.pendingp);
		send_buffer.pendingp=0;
	}
	if(send_buffer.wptr>0 && send_buffer.wptr%2==0) send_buffer.wptr--;
}

void send_header(struct ir_remote *remote)
{
	if(has_header(remote))
	{
		send_pulse(remote->phead);
		send_space(remote->shead);
	}
}

void send_foot(struct ir_remote *remote)
{
	if(has_foot(remote))
	{
		send_space(remote->sfoot);
		send_pulse(remote->pfoot);
	}
}

void send_lead(struct ir_remote *remote)
{
	if(remote->plead!=0)
	{
		send_pulse(remote->plead);
	}
}

void send_trail(struct ir_remote *remote)
{
	if(remote->ptrail!=0)
	{
		send_pulse(remote->ptrail);
	}
}

void send_data(struct ir_remote *remote,ir_code data,int bits,int done)
{
	int i;
	int all_bits = bit_count(remote);
	int toggle_bit_mask_bits = bits_set(remote->toggle_bit_mask);
	ir_code mask;
	
	data=reverse(data,bits);
	if(is_rcmm(remote))
	{
		mask=(ir_code)1<<(all_bits-1-done);
		if(bits%2 || done%2)
		{
			return;
		}
		for(i=0;i<bits;i+=2,mask>>=2)
		{
			switch(data&3)
			{
			case 0:
				send_pulse(remote->pzero);
				send_space(remote->szero);
				break;
			/* 2 and 1 swapped due to reverse() */
			case 2:
				send_pulse(remote->pone);
				send_space(remote->sone);
				break;
			case 1:
				send_pulse(remote->ptwo);
				send_space(remote->stwo);
				break;
			case 3:
				send_pulse(remote->pthree);
				send_space(remote->sthree);
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
		for(i = 0; i < bits; i += 4)
		{
			ir_code nibble;

			nibble = reverse(data & 0xf, 4);
			send_pulse(remote->pzero);
			send_space((unsigned long)(remote->szero + nibble*remote->sone));
			data >>= 4;
		}
		return;
	}

	mask=((ir_code) 1)<<(all_bits-1-done);
	for(i=0;i<bits;i++,mask>>=1)
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
					send_space(2*remote->sone);
					send_pulse(2*remote->pone);
				}
				else
				{
					send_space(remote->sone);
					send_pulse(remote->pone);
				}
			}
			else if(is_space_first(remote))
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
			if(mask&remote->rc6_mask)
			{
				send_pulse(2*remote->pzero);
				send_space(2*remote->szero);
			}
			else if(is_space_first(remote))
			{
				send_space(remote->szero);
				send_pulse(remote->pzero);
			}
			else
			{
				send_pulse(remote->pzero);
				send_space(remote->szero);
			}
		}
		data=data>>1;
	}
}

void send_pre(struct ir_remote *remote)
{
	if(has_pre(remote))
	{
		send_data(remote,remote->pre_data,remote->pre_data_bits,0);
		if(remote->pre_p>0 && remote->pre_s>0)
		{
			send_pulse(remote->pre_p);
			send_space(remote->pre_s);
		}
	}
}

void send_post(struct ir_remote *remote)
{
	if(has_post(remote))
	{
		if(remote->post_p>0 && remote->post_s>0)
		{
			send_pulse(remote->post_p);
			send_space(remote->post_s);
		}
		send_data(remote,remote->post_data,remote->post_data_bits,
			  remote->pre_data_bits+remote->bits);
	}
}

void send_repeat(struct ir_remote *remote)
{
	send_lead(remote);
	send_pulse(remote->prepeat);
	send_space(remote->srepeat);
	send_trail(remote);
}

void send_code(struct ir_remote *remote,ir_code code, int repeat)
{
	if(!repeat || !(remote->flags&NO_HEAD_REP))
		send_header(remote);
	send_lead(remote);
	send_pre(remote);
	send_data(remote,code,remote->bits,remote->pre_data_bits);
	send_post(remote);
	send_trail(remote);
	if(!repeat || !(remote->flags&NO_FOOT_REP))
		send_foot(remote);
	
	if(!repeat &&
	   remote->flags&NO_HEAD_REP &&
	   remote->flags&CONST_LENGTH)
	{
		send_buffer.sum-=remote->phead+remote->shead;
	}
}

void send_signals(lirc_t *signals, int n)
{
	int i;
	
	for(i=0; i<n; i++)
	{
		add_send_buffer(signals[i]);
	}
}

WINLIRC_API int init_send(struct ir_remote *remote,struct ir_ncode *code, int repeats)
{
	int repeat=0;
	
	if(is_grundig(remote) ||  is_goldstar(remote) || is_serial(remote) || is_bo(remote))
	{
		return(0);
	}
	clear_send_buffer();
	if(is_biphase(remote))
	{
		send_buffer.is_biphase=1;
	}

	remote->repeat_countdown = max(remote->min_repeat,repeats);
	
 init_send_loop:
	if(repeat && has_repeat(remote))
	{
		if(remote->flags&REPEAT_HEADER && has_header(remote))
		{
			send_header(remote);
		}
		send_repeat(remote);
	}
	else
	{
		if(!is_raw(remote))
		{	
			ir_code next_code;
			
			if(code->transmit_state == NULL)
			{
				next_code = code->code;
			}
			else
			{
				next_code = code->transmit_state->code;
			}

			send_code(remote, next_code, repeat);

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
			if(code->signals==NULL)
			{
				return 0;
			}
			
			send_signals(code->signals, code->length);
		}
	}
	sync_send_buffer();
	if(bad_send_buffer())
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
	if(code->next != NULL)
	{
		if(code->transmit_state == NULL)
		{
			code->transmit_state = code->next;
		}
		else
		{
			code->transmit_state = code->transmit_state->next;
			if(is_xmp(remote) && code->transmit_state == NULL)
			{
				code->transmit_state = code->next;
			}
		}
	}

	send_space(remote->min_remaining_gap);
	flush_send_buffer();

	if(remote->repeat_countdown>0 || code->transmit_state != NULL )
	{
		if(code->next == NULL || code->transmit_state == NULL)
		{
			remote->repeat_countdown--;
			repeat = 1;
		}
		
		send_buffer.sum	= 0;
		
		goto init_send_loop;
	}

	send_buffer.data=send_buffer._data;

	if(!check_send_buffer())
	{
		return 0;
	}
	
	return 1;
}