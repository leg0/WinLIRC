#include <winlirc/winlirc_api.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "remote.h"
#include "config.h"

#include "wl_string.h"
#include <charconv>
#include <filesystem>

static constexpr size_t LINE_LEN = 1024;
static constexpr uint32_t MAX_INCLUDES = 10;
static constexpr char whitespace[] = " \t";
static int line;
static int parse_error;

static ir_remote * read_config_recursive(FILE *f, winlirc::istring_view name, int depth);

template <typename Int, typename Char, typename CharTraits>
requires(std::is_integral_v<Int>)
static Int s_str_to_int(std::basic_string_view<Char, CharTraits> s)
{
    Int        out;
    auto       end  = s.data() + s.size();
    auto const base = [&]() {
        if (s.starts_with("0x"))
        {
            s.remove_prefix(2);
            return 16;
        }
        else
            return 10;
    }();
    auto res = std::from_chars(s.data(), end, out, base);
    if (res.ec == std::errc{} && res.ptr == end)
        return out;

    parse_error = 1;
    return Int{};
}

template <typename Char, typename CharTraits>
static ir_code s_strtocode(std::basic_string_view<Char, CharTraits> val)
{
    return s_str_to_int<ir_code>(val);
}

template <typename Char, typename CharTraits>
static unsigned long s_strtoul(std::basic_string_view<Char, CharTraits> val)
{
    return s_str_to_int<unsigned long>(val);
}

template <typename Char, typename CharTraits>
static int s_strtoi(std::basic_string_view<Char, CharTraits> val)
{
    return s_str_to_int<int>(val);
}

template <typename Char, typename CharTraits>
static unsigned int s_strtoui(std::basic_string_view<Char, CharTraits> val)
{
    return s_str_to_int<unsigned int>(val);
}

template <typename Char, typename CharTraits>
static lirc_t s_strtolirc_t(std::basic_string_view<Char, CharTraits> val)
{
    return s_str_to_int<lirc_t>(val);
}

int checkMode(int is_mode, int c_mode, char const* error)
{
	if (is_mode!=c_mode)
	{
		parse_error=1;
		return(0);
	}
	return(1);
}

int addSignal(std::vector<lirc_t>& signals, winlirc::istring_view val)
{
	auto t=s_strtolirc_t(val);
	if(parse_error) return(0);
	signals.push_back(t);
	return(1);
}

ir_ncode* defineCode(winlirc::istring_view key, winlirc::istring_view val, ir_ncode* code)
{
	*code = ir_ncode{};
	code->name = std::string{ begin(key), end(key) };
	code->code = s_strtocode(val);
	return code;
}

ir_code_node* defineNode(ir_ncode* code, winlirc::istring_view val)
{
	auto node = std::make_unique<ir_code_node>();
	auto const res = node.get();
	node->code=s_strtocode(val);
	node->next=nullptr;

	if (code->current == nullptr)
	{
		code->current = node.get();
		code->next = std::move(node);
	}
	else
	{
		code->current = node.get();
		code->current->next = std::move(node);
	}
	return res;
}

int parseFlags(winlirc::istring_view val)
{
    int flags = 0;
    while (!val.empty())
    {
        auto flagName = strtok(val, "|");
        auto flagIt   = std::find_if(std::begin(all_flags), std::end(all_flags), [&](auto& fp) {
            return fp.name == flagName;
        });
        if (
          flagIt == std::end(all_flags) ||
          (flagIt->flag & IR_PROTOCOL_MASK) && (flags & IR_PROTOCOL_MASK))
        {
            parse_error = 1;
            return 0;
        }
        flags = flags | flagIt->flag;
    }
    return flags;
}

int defineRemote(winlirc::istring_view key, winlirc::istring_view val, winlirc::istring_view val2, ir_remote *rem)
{
	if ("name" == key){
		rem->name.assign(val.begin(), val.end());	
		return(1);
	}
	else if ("bits" == key){
		rem->bits=s_strtoi(val);
		return(1);
	}
	else if ("flags" == key){
		rem->flags|=parseFlags(val);
		return(1);
	}
	else if ("eps" == key){
		rem->eps=s_strtoi(val);
		return(1);
	}
	else if ("aeps" == key){
		rem->aeps=s_strtoi(val);
		return(1);
	}
	else if ("plead" == key){
		rem->plead=s_strtolirc_t(val);
		return(1);
	}
	else if ("ptrail" == key){
		rem->ptrail=s_strtolirc_t(val);
		return(1);
	}
	else if ("pre_data_bits" == key){
		rem->pre_data_bits=s_strtoi(val);
		return(1);
	}
	else if ("pre_data" == key){
		rem->pre_data=s_strtocode(val);
		return(1);
	}
	else if ("post_data_bits" == key){
		rem->post_data_bits=s_strtoi(val);
		return(1);
	}
	else if ("post_data" == key){
		rem->post_data=s_strtocode(val);
		return(1);
	}
	else if ("gap" == key){
		if(!val2.empty())
		{
			rem->gap2=s_strtoul(val2);
		}
		rem->gap=s_strtoul(val);
		return(!val2.empty() ? 2:1);
	}
	else if ("repeat_gap" == key){
		rem->repeat_gap=s_strtoul(val);
		return(1);
	}
	/* obsolete: use toggle_bit_mask instead */
	else if ("toggle_bit" == key){
		rem->toggle_bit = s_strtoi(val);
		return 1;
	}
	else if ("toggle_bit_mask" == key){
		rem->toggle_bit_mask = s_strtocode(val);
		return 1;
	}
	else if ("toggle_mask" == key){
		rem->toggle_mask=s_strtocode(val);
		return(1);
	}
	else if ("rc6_mask" == key){
		rem->rc6_mask=s_strtocode(val);
		return(1);
	}
	else if ("ignore_mask" == key){
		rem->ignore_mask=s_strtocode(val);
		return(1);
	}
	/* obsolete name */
	else if ("repeat_bit" == key){
		rem->toggle_bit=s_strtoi(val);
		return(1);
	}
	else if ("suppress_repeat" == key){
		//rem->suppress_repeat=s_strtoi(val);	//TODO support this per remote
		return(1);
	}
	else if ("min_repeat" == key){
		rem->min_repeat=s_strtoi(val);
		return(1);
	}
	else if ("min_code_repeat" == key){
		rem->min_code_repeat=s_strtoi(val);
		return(1);
	}
	else if ("frequency" == key){
		rem->freq=s_strtoui(val);
		return(1);
	}
	else if ("duty_cycle" == key){
		rem->duty_cycle=s_strtoui(val);
		return(1);
	}
	else if ("baud" == key){
		rem->baud=s_strtoui(val);
		return(1);
	}
	else if ("serial_mode" == key){
		if(val[0]<'5' || val[0]>'9')
		{
			parse_error=1;
			return 0;
		}
		rem->bits_in_byte=val[0]-'0';
		switch(toupper(val[1]))
		{
		case 'N':
			rem->parity = IR_PARITY_NONE;
			break;
		case 'E':
			rem->parity = IR_PARITY_EVEN;
			break;
		case 'O':
			rem->parity = IR_PARITY_ODD;
			break;
		default:
			parse_error=1;
			return 0;
		}
		if(val.substr(2) == "1.5")
		{
			rem->stop_bits=3;
		}
		else
		{
			rem->stop_bits=s_strtoui(val.substr(2))*2;
		}
		return(1);
	}
	else if (!val2.empty())
	{
		if ("header" == key){
			rem->phead=s_strtolirc_t(val);
			rem->shead=s_strtolirc_t(val2);
			return(2);
		}
		else if ("three" == key){
			rem->pthree=s_strtolirc_t(val);
			rem->sthree=s_strtolirc_t(val2);
			return(2);
		}
		else if ("two" == key){
			rem->ptwo=s_strtolirc_t(val);
			rem->stwo=s_strtolirc_t(val2);
			return(2);
		}
		else if ("one" == key){
			rem->pone=s_strtolirc_t(val);
			rem->sone=s_strtolirc_t(val2);
			return(2);
		}
		else if ("zero" == key){
			rem->pzero=s_strtolirc_t(val);
			rem->szero=s_strtolirc_t(val2);
			return(2);
		}
		else if ("foot" == key){
			rem->pfoot=s_strtolirc_t(val);
			rem->sfoot=s_strtolirc_t(val2);
			return(2);
		}
		else if ("repeat" == key){
			rem->prepeat=s_strtolirc_t(val);
			rem->srepeat=s_strtolirc_t(val2);
			return(2);
		}
		else if ("pre" == key){
			rem->pre_p=s_strtolirc_t(val);
			rem->pre_s=s_strtolirc_t(val2);
			return(2);
		}
		else if ("post" == key){
			rem->post_p=s_strtolirc_t(val);
			rem->post_s=s_strtolirc_t(val2);
			return(2);
		}
	}
	parse_error=1;
	return(0);
}

static int sanityChecks(ir_remote *rem)
{
	if (rem->name.empty())
	{
		return 0;
	}

	if(is_raw(rem)) return 1;

	if((rem->pre_data&gen_mask(rem->pre_data_bits)) != rem->pre_data)
	{
		rem->pre_data &= gen_mask(rem->pre_data_bits);
	}
	if((rem->post_data&gen_mask(rem->post_data_bits)) != rem->post_data)
	{
		rem->post_data &= gen_mask(rem->post_data_bits);
	}
	for (auto codes = rem->codes.begin(); codes != rem->codes.end(); codes++)
	{
		if ((codes->code & gen_mask(rem->bits)) != codes->code)
		{
			codes->code &= gen_mask(rem->bits);
		}
		for (auto node = codes->next.get(); node != nullptr; node = node->next.get())
		{
			if ((node->code & gen_mask(rem->bits)) != node->code)
			{
				node->code &= gen_mask(rem->bits);
			}
		}
	}

	return 1;
}

ir_remote* sort_by_bit_count(ir_remote *remotes)
{
	std::vector<ir_remote*> v;
	for (auto rem = remotes; rem != nullptr; rem = rem->next.get())
	{
		v.push_back(rem);
	}
	std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return bit_count(a) < bit_count(b); });
	for (ir_remote* rem : v)
	{
		rem->next.release();
	}
	for (size_t i = 1; i < v.size(); ++i)
	{
		v[i - 1]->next.reset(v[i]);
	}
	v.back()->next.reset();
	return v[0];
}

static winlirc::istring_view lirc_parse_include(winlirc::istring_view s)
{
    s = winlirc::rtrim(s, whitespace);
    if (!s.empty() && (s.front() == '"' && s.back() == '"' || s.front() == '<' && s.back() == '>'))
    {
        s.remove_prefix(1);
        s.remove_suffix(1);
        return s;
    }
    return {};
}

static winlirc::istring lirc_parse_relative(winlirc::istring_view child, winlirc::istring_view current)
{
	if (current.empty())
		return static_cast<winlirc::istring>(child);

	std::filesystem::path p{ child };
	if (p.is_absolute())
		return static_cast<winlirc::istring>(child);

	auto cur = std::filesystem::path { current }.remove_filename() / child;
	return cur.string().c_str();
}

ir_remote * read_config(FILE *f, const char *name)
{
	return read_config_recursive(f, name, 0);
}

static ir_remote * read_config_recursive(FILE *f, winlirc::istring_view name, int depth)
{
    char bufx[LINE_LEN + 1];
	winlirc::istring_view key, val, val2;
	int argc;
	ir_remote *top_rem=nullptr,*rem=nullptr;
	std::vector<ir_ncode> codes_list,raw_codes;
	std::vector<lirc_t> signals;
	ir_ncode raw_code{};
	ir_ncode name_code{};
	ir_ncode *code;
	int mode=ID_none;

	line=0;
	parse_error=0;

	while(fgets(bufx,LINE_LEN,f)!=nullptr)
	{
        winlirc::istring_view buf{ bufx };
		line++;
        if (buf.size() == LINE_LEN && buf.back() != '\n')
		{
			parse_error=1;
			break;
		}

		if (!buf.empty() && buf.back() == '\n')
            buf.remove_suffix(1);
        if (!buf.empty() && buf.back() == '\r')
            buf.remove_suffix(1);
		/* ignore comments */
		if(!buf.empty() && buf[0]=='#')
			continue;

		key=strtok(buf, whitespace);
		/* ignore empty lines */
		if(key.empty()) continue;
		val=strtok(buf, whitespace);
		if(!val.empty()){
			val2=strtok(buf, whitespace);

			if ("include" == key){
				if (depth > MAX_INCLUDES) {
					parse_error=-1;
					break;
				}

				auto childName = lirc_parse_include(val);
				if (childName.empty()){
					parse_error=-1;
					break;
				}

				auto fullPath = lirc_parse_relative(childName, name);
				if (fullPath.empty()) {
					parse_error=-1;
					break;
				}

				FILE* childFile;
				auto const openResult = fopen_s(&childFile, fullPath.c_str(), "r");
				if (childFile != nullptr)
				{
					int save_line = line;

					if (!top_rem){
						/* create first remote */
						rem = read_config_recursive(childFile, fullPath, depth + 1);
						if(rem != (void *) -1 && rem != nullptr) {
							top_rem = rem;
						} else {
							rem = nullptr;
						}
					}else{
						/* create new remote */

						rem->next.reset(read_config_recursive(childFile, fullPath, depth + 1));
						rem=rem->next.get();
					}
					fclose(childFile);
					line = save_line;
				}
			}else if ("begin" == key){
				if ("codes" == val){
					/* init codes mode */

					if (!checkMode(mode, ID_remote,
						"begin codes")) break;
					if (!rem->codes.empty()){
						parse_error=1;
						break;
					}

					codes_list.clear();
					codes_list.reserve(30);
					mode=ID_codes;
				}else if("raw_codes" == val){
					/* init raw_codes mode */

					if(!checkMode(mode, ID_remote,
						"begin raw_codes")) break;
					if (!rem->codes.empty()){

						parse_error=1;
						break;
					}
					set_protocol(rem, RAW_CODES);
					raw_code.code=0;
					raw_codes.clear();
					raw_codes.reserve(30);
					mode=ID_raw_codes;
				}else if("remote" == val){
					/* create new remote */
					
					if(!checkMode(mode, ID_none,
						"begin remote")) break;
					mode=ID_remote;
					if (!top_rem){
						/* create first remote */
						
						rem = top_rem = new ir_remote{};
					}else{
						/* create new remote */
						
						rem->next.reset(new ir_remote{});
						rem=rem->next.get();
					}
				}else if(mode==ID_codes){
					code=defineCode(key, val, &name_code);
					while(!parse_error && !val2.empty())
					{
						struct ir_code_node *node;

						if(val2[0]=='#') break; /* comment */
						node=defineNode(code, val2);
						val2=strtok(buf, whitespace);
					}
					code->current=nullptr;
					codes_list.push_back(std::move(*code));
				}else{
					parse_error=1;
				}
			}else if ("end" == key){

				if ("codes" == val){
					/* end Codes mode */
					if (!checkMode(mode, ID_codes,
						"end codes")) break;
					rem->codes = std::move(codes_list);
					codes_list.clear();
					mode=ID_remote;     /* switch back */

				}else if("raw_codes" == val){
					/* end raw codes mode */


					if(mode==ID_raw_name){
						raw_code.signals=signals;
						if(raw_code.length()%2==0)
						{
							parse_error=1;
						}
						raw_codes.push_back(std::move(raw_code));
						mode=ID_raw_codes;
					}
					if(!checkMode(mode,ID_raw_codes,
						"end raw_codes")) break;
					rem->codes = std::move(raw_codes);
					raw_codes.clear();
					mode=ID_remote;     /* switch back */
				}else if("remote" == val){
					/* end remote mode */
					/* print_remote(rem); */
					if (!checkMode(mode,ID_remote,
						"end remote")) break;
					if(!sanityChecks(rem)) {
						parse_error=1;
						break;
					}

					/* not really necessary because we
					clear the alloced memory */
					rem->next=nullptr;
					rem->last_code=nullptr;
					mode=ID_none;     /* switch back */
				}else if(mode==ID_codes){
					code=defineCode(key, val, &name_code);
					while(!parse_error && !val2.empty())
					{
						if(val2[0]=='#') break; /* comment */
						ir_code_node* node=defineNode(code, val2);
						val2=strtok(buf, whitespace);
					}
					code->current=nullptr;
					codes_list.push_back(std::move(*code));
				}else{
					parse_error=1;
				}
			} else {
				switch (mode){
	case ID_remote:
		argc=defineRemote(key, val, val2, rem);
		break;
	case ID_codes:
		code=defineCode(key, val, &name_code);
		while(!parse_error && !val2.empty())
		{
			if(val2[0]=='#') break; /* comment */
			ir_code_node* node=defineNode(code, val2);
			val2=strtok(buf, whitespace);
		}
		code->current=nullptr;
		codes_list.push_back(std::move(*code));
		break;
	case ID_raw_codes:
	case ID_raw_name:
		if("name" == key){
			if(mode==ID_raw_name)
			{
				raw_code.signals = std::move(signals);
				if(raw_code.length()%2==0)
				{
					parse_error=1;
				}
				raw_codes.push_back(std::move(raw_code));
			}
			raw_code.name = std::string{ begin(val), end(val) };
			raw_code.code++;
			signals.clear();
			signals.reserve(50);
			mode=ID_raw_name;
		}else{
			if(mode==ID_raw_codes)
			{
				parse_error=1;
				break;
			}
			if(!addSignal(signals, key)) break;
			if(!addSignal(signals, val)) break;
			if (!val2.empty()){
				if (!addSignal(signals, val2)){
					break;
				}
			}
			while (true){
				auto val=strtok(buf, whitespace);
				if (val.empty() || !addSignal(signals, val)) break;
			}
		}
		break;
				}
			}
		}else if(mode==ID_raw_name){
			if(!addSignal(signals, key)){
				break;
			}
		}else{
			parse_error=1;
			break;
		}
		if (parse_error){
			break;
		}
	}
	if(mode!=ID_none)
	{
		switch(mode)
		{
		case ID_raw_name:
			if (raw_code.name != std::nullopt)
			{
				raw_code.name->clear();
				signals.clear();
			}
		case ID_raw_codes:
			rem->codes = std::move(raw_codes);
			break;
		case ID_codes:
			rem->codes = std::move(codes_list);
			break;
		}
		if(!parse_error)
		{
			parse_error=1;
		}
	}
	if (parse_error){
		static int print_error = 1;

		if(print_error) {

		}
		delete top_rem;
		if(depth == 0) print_error = 1;
		return nullptr;
	}
	/* kick reverse flag */
	/* handle RC6 flag to be backwards compatible: previous RC-6
	config files did not set rc6_mask */
	rem=top_rem;
	while(rem!=nullptr)
	{
		if((!is_raw(rem)) && rem->flags&REVERSE)
		{
			if(has_pre(rem))
			{
				rem->pre_data=reverse(rem->pre_data,
					rem->pre_data_bits);
			}
			if(has_post(rem))
			{
				rem->post_data=reverse(rem->post_data,
					rem->post_data_bits);
			}
			for (auto& c : rem->codes)
			{
				c.code = reverse(c.code, rem->bits);
			}
			rem->flags=rem->flags&(~REVERSE);
			rem->flags=rem->flags|COMPAT_REVERSE;
			/* don't delete the flag because we still need
			it to remain compatible with older versions
			*/
		}
		if(rem->flags&RC6 && rem->rc6_mask==0 && rem->toggle_bit>0)
		{
			int all_bits=bit_count(rem);

			rem->rc6_mask=((ir_code) 1)<<(all_bits-rem->toggle_bit);
		}
		if(rem->toggle_bit > 0)
		{
			int all_bits=bit_count(rem);

			if(!has_toggle_bit_mask(rem))
			{
				rem->toggle_bit_mask=((ir_code) 1)<<(all_bits-rem->toggle_bit);
			}
			rem->toggle_bit = 0;
		}
		if(has_toggle_bit_mask(rem))
		{
			if(!is_raw(rem) && !rem->codes.empty())
			{
				rem->toggle_bit_mask_state = (rem->codes[0].code & rem->toggle_bit_mask);
				if(rem->toggle_bit_mask_state)
				{
					/* start with state set to 0 for backwards compatibility */
					rem->toggle_bit_mask_state ^= rem->toggle_bit_mask;
				}
			}
		}
		if(is_serial(rem))
		{
			if(rem->baud>0)
			{
				lirc_t base=1000000/rem->baud;
				if(rem->pzero==0 && rem->szero==0)
				{
					rem->pzero=base;
				}
				if(rem->pone==0 && rem->sone==0)
				{
					rem->sone=base;
				}
			}
			if(rem->bits_in_byte==0)
			{
				rem->bits_in_byte=8;
			}
		}
		if(rem->min_code_repeat>0)
		{
			if(!has_repeat(rem) ||
				rem->min_code_repeat>rem->min_repeat)
			{
				rem->min_code_repeat = 0;
			}
		}
		rem=rem->next.get();
	}

	top_rem = sort_by_bit_count(top_rem);
#       if defined(DEBUG) && !defined(DAEMONIZE)
	/*fprint_remotes(stderr, top_rem);*/
#       endif
	return (top_rem);
}
