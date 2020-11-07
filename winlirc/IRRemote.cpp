#include "constants.h"
#include "ir_remote.h"
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <limits.h>

struct ir_remote *decoding		= nullptr;
struct ir_remote *last_remote	= nullptr;
struct ir_remote *repeat_remote	= nullptr;
struct ir_ncode *repeat_code	= nullptr;

WINLIRC_API int winlirc_map_code(struct ir_remote *remote,
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

WINLIRC_API void winlirc_map_gap(
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
	struct ir_ncode *found;
	
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
	if(remote->codes.ptr!=nullptr)
	{
		for (auto& c : remote->codes)
		{
			ir_code next_all;

			next_all = gen_ir_code(remote, remote->pre_data,
					       get_ir_code(&c, c.current),
					       remote->post_data);
			if(match_ir_code(remote, next_all, all))
			{
				found_code=1;
				if(c.next!=nullptr)
				{
					if(c.current==nullptr)
					{
						c.current=c.next;
					}
					else
					{
						c.current=
							c.current->next;
					}
				}
				if(!have_code)
				{
					found=&c;
					if(c.current==nullptr)
					{
						have_code=1;
					}
				}
			}
			else
			{
				/* find longest matching sequence */
				struct ir_code_node *search;
				
				search = c.next;
				if(search == nullptr ||
					(c.next != nullptr && c.current == nullptr))
				{
					c.current=nullptr;
				}
				else
				{
					int sequence_match = 0;
					
					while(search != c.current->next)
					{
						struct ir_code_node *prev, *next;
						int flag = 1;
						
						prev = nullptr; /* means c.code */
						next = search;
						while(next != c.current)
						{
							if(get_ir_code(&c, prev) != get_ir_code(&c, next))
							{
								flag = 0;
								break;
							}
							prev = get_next_ir_code_node(&c, prev);
							next = get_next_ir_code_node(&c, next);
						}
						if(flag == 1)
						{
							next_all = gen_ir_code(remote, remote->pre_data,
									       get_ir_code(&c, prev),
									       remote->post_data);
							if(match_ir_code(remote, next_all, all))
							{
								c.current = get_next_ir_code_node(&c, prev);
								sequence_match = 1;
								found_code=1;
								if(!have_code)
								{
									found=&c;
								}
								break;
							}
						}
						search = search->next;
					}
					if(!sequence_match) c.current = nullptr;
				}
			}
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

static unsigned long long set_code(ir_remote *remote, ir_ncode *found,
			    ir_code toggle_bit_mask_state,int repeat_flag,
			    lirc_t min_remaining_gap, lirc_t max_remaining_gap)
{
	unsigned long long code;
	static struct ir_remote *last_decoded = nullptr;

	auto const current = std::chrono::steady_clock::now();

	using namespace std::chrono_literals;

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

static int write_message(char *buffer, size_t size, const char *remote_name,
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

WINLIRC_API bool winlirc_decodeCommand(rbuf* prec_buffer, hardware const* phw, struct ir_remote *remotes, char *out, size_t out_size)
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
		
		if(hw.decode_func(prec_buffer, &hw,remote,&pre,&code,&post,&repeat_flag,
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
				for (auto& scan_ncode : scan->codes)
				{
					scan_ncode.current = nullptr;
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

WINLIRC_API ir_code get_ir_code(struct ir_ncode* ncode, struct ir_code_node* node)
{
	if (ncode->next && node != nullptr) return node->code;
	return ncode->code;
}

WINLIRC_API struct ir_code_node* get_next_ir_code_node(struct ir_ncode* ncode, struct ir_code_node* node)
{
	if (node == nullptr) return ncode->next;
	return node->next;
}

WINLIRC_API int bit_count(ir_remote const* remote)
{
	return remote->pre_data_bits +
		remote->bits +
		remote->post_data_bits;
}

WINLIRC_API int has_repeat(ir_remote const* remote)
{
	if (remote->prepeat > 0 && remote->srepeat > 0) return(1);
	else return(0);
}

WINLIRC_API void set_protocol(ir_remote* remote, int protocol)
{
	remote->flags &= ~(IR_PROTOCOL_MASK);
	remote->flags |= protocol;
}

WINLIRC_API int is_raw(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == RAW_CODES) return(1);
	else return(0);
}

WINLIRC_API int is_space_enc(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == SPACE_ENC) return(1);
	else return(0);
}

WINLIRC_API int is_space_first(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == SPACE_FIRST) return(1);
	else return(0);
}

WINLIRC_API int is_rc5(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == RC5) return(1);
	else return(0);
}

WINLIRC_API int is_rc6(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == RC6 ||
		remote->rc6_mask) return(1);
	else return(0);
}

WINLIRC_API int is_biphase(ir_remote const* remote)
{
	if (is_rc5(remote) || is_rc6(remote)) return(1);
	else return(0);
}

WINLIRC_API int is_rcmm(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == RCMM) return(1);
	else return(0);
}

WINLIRC_API int is_goldstar(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == GOLDSTAR) return(1);
	else return(0);
}

WINLIRC_API int is_grundig(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == GRUNDIG) return(1);
	else return(0);
}

WINLIRC_API int is_bo(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == BO) return(1);
	else return(0);
}

WINLIRC_API int is_serial(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == SERIAL) return(1);
	else return(0);
}

WINLIRC_API int is_xmp(ir_remote const* remote)
{
	if ((remote->flags & IR_PROTOCOL_MASK) == XMP) return(1);
	else return(0);
}

WINLIRC_API int is_const(ir_remote const* remote)
{
	if (remote->flags & CONST_LENGTH) return(1);
	else return(0);
}

WINLIRC_API int has_repeat_gap(ir_remote const* remote)
{
	if (remote->repeat_gap > 0) return(1);
	else return(0);
}

WINLIRC_API int has_pre(ir_remote const* remote)
{
	if (remote->pre_data_bits > 0) return(1);
	else return(0);
}

WINLIRC_API int has_post(ir_remote const* remote)
{
	if (remote->post_data_bits > 0) return(1);
	else return(0);
}

WINLIRC_API int has_header(ir_remote const* remote)
{
	if (remote->phead > 0 && remote->shead > 0) return(1);
	else return(0);
}

WINLIRC_API int has_foot(ir_remote const* remote)
{
	if (remote->pfoot > 0 && remote->sfoot > 0) return(1);
	else return(0);
}

WINLIRC_API int has_toggle_bit_mask(ir_remote const* remote)
{
	if (remote->toggle_bit_mask > 0) return(1);
	else return(0);
}

WINLIRC_API int has_ignore_mask(ir_remote const* remote)
{
	if (remote->ignore_mask > 0) return(1);
	else return(0);
}

WINLIRC_API int has_toggle_mask(ir_remote const* remote)
{
	if (remote->toggle_mask > 0) return(1);
	else return(0);
}

WINLIRC_API lirc_t min_gap(ir_remote const* remote)
{
	if (remote->gap2 != 0 && remote->gap2 < remote->gap)
	{
		return remote->gap2;
	}
	else
	{
		return remote->gap;
	}
}

WINLIRC_API lirc_t max_gap(ir_remote const* remote)
{
	if (remote->gap2 > remote->gap)
	{
		return remote->gap2;
	}
	else
	{
		return remote->gap;
	}
}

WINLIRC_API ir_code gen_ir_code(ir_remote const* remote, ir_code pre, ir_code code, ir_code post)
{
	ir_code all;

	all = (pre & gen_mask(remote->pre_data_bits));
	all <<= remote->bits;
	all |= is_raw(remote) ? code : (code & gen_mask(remote->bits));
	all <<= remote->post_data_bits;
	all |= post & gen_mask(remote->post_data_bits);

	return all;
}

WINLIRC_API int match_ir_code(ir_remote const* remote, ir_code a, ir_code b)
{
	return ((remote->ignore_mask | a) == (remote->ignore_mask | b) || (remote->ignore_mask | a) == (remote->ignore_mask | (b ^ remote->toggle_bit_mask)));
}

WINLIRC_API int expect(ir_remote const* remote, lirc_t delta, lirc_t exdelta)
{
	int aeps = remote->aeps;

	if (abs(exdelta - delta) <= exdelta * remote->eps / 100 ||
		abs(exdelta - delta) <= aeps)
		return 1;
	return 0;
}

WINLIRC_API int expect_at_least(ir_remote const* remote,
	lirc_t delta, lirc_t exdelta)
{
	int aeps = remote->aeps;

	if (delta + exdelta * remote->eps / 100 >= exdelta ||
		delta + aeps >= exdelta)
	{
		return 1;
	}
	return 0;
}

WINLIRC_API int expect_at_most(ir_remote const* remote,
	lirc_t delta, lirc_t exdelta)
{
	int aeps = remote->aeps;

	if (delta <= exdelta + exdelta * remote->eps / 100 ||
		delta <= exdelta + aeps)
	{
		return 1;
	}
	return 0;
}

WINLIRC_API unsigned get_freq(ir_remote const* remote)
{
	return remote->freq;
}

WINLIRC_API void set_freq(ir_remote* remote, unsigned freq)
{
	remote->freq = freq;
}

WINLIRC_API unsigned get_duty_cycle(ir_remote const* remote)
{
	return remote->duty_cycle;
}

WINLIRC_API void set_duty_cycle(ir_remote* remote, unsigned duty_cycle)
{
	remote->duty_cycle = duty_cycle;
}
