/*      $Id: dump_config.c,v 5.23 2009/02/12 21:14:48 lirc Exp $      */

/****************************************************************************
 ** dump_config.c ***********************************************************
 ****************************************************************************
 *
 * dump_config.c - dumps data structures into file
 *
 * Copyright (C) 1998 Pablo d'Angelo <pablo@ag-trek.allgaeu.org>
 *
 */ 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "dump_config.h"
#include "../../winlirc/config.h"
#include "../../winlirc/ir_remote.h"
#include "../../winlirc/IRRemote.h"

#include <algorithm>

#define VERSION "0.9.0"

void fprint_comment(FILE* f, ir_remote const* rem, hardware const& hw) noexcept
{
	time_t timet=time(NULL);
	tm* tmp=localtime(&timet);
	fprintf(f,
		"#\n"
		"# this config file was automatically generated\n"
		"# using lirc-%s(%s) on %s"
		"#\n"
		"# contributed by \n"
		"#\n"
		"# brand:                       %s\n"
		"# model no. of remote control: \n"
		"# devices being controlled by this remote:\n"
		"#\n\n", VERSION, &hw.name[0], asctime(tmp),
		rem->name.c_str());
}

void fprint_flags(FILE* f, int flags) noexcept
{
	auto it = std::find_if(std::begin(all_flags), std::end(all_flags), [=](auto& x) {
		return (x.flag & flags) == x.flag;
	});
	if (it != std::end(all_flags))
	{
		fprintf(f, "  flags %s", it->name);
		for (++it; it != std::end(all_flags); ++it)
		{
			if ((it->flag & flags) == it->flag)
				fprintf(f, "|%s", it->name);
		}
		fprintf(f, "\n");
	}
}

void fprint_remotes(FILE* f, ir_remote const* all, hardware const& hw) {

	while (all)
	{
		fprint_remote(f, all, hw);
		fprintf(f, "\n\n");
		all = all->next.get();
	}
}

void fprint_remote_gap(FILE* f, ir_remote const* rem) noexcept
{
	if(rem->gap2 != 0)
	{
		fprintf(f, "  gap          %lu %lu\n",
			(unsigned long) rem->gap,
			(unsigned long) rem->gap2);
	}
	else
	{
		fprintf(f, "  gap          %lu\n", static_cast<unsigned long>(rem->gap));
	}
}

void fprint_remote_head(FILE* f, ir_remote const* rem)
{
	fprintf(f, "begin remote\n\n");
	fprintf(f, "  name  %s\n",rem->name.c_str());
	if(!is_raw(rem))
	{
		fprintf(f, "  bits        %5d\n",rem->bits);
	}
	fprint_flags(f,rem->flags);
	fprintf(f, "  eps         %5d\n",rem->eps);
	fprintf(f, "  aeps        %5d\n\n",rem->aeps);
	if(!is_raw(rem))
	{
		if(has_header(rem))
		{
			fprintf(f, "  header      %5lu %5lu\n",
				(unsigned long) rem->phead,
				(unsigned long) rem->shead);
		}
		if(rem->pthree!=0 || rem->sthree!=0)
			fprintf(f, "  three       %5lu %5lu\n",
				(unsigned long) rem->pthree,
				(unsigned long) rem->sthree);
		if(rem->ptwo!=0 || rem->stwo!=0)
			fprintf(f, "  two         %5lu %5lu\n",
				(unsigned long) rem->ptwo,
				(unsigned long)  rem->stwo);
		fprintf(f, "  one         %5lu %5lu\n",
			(unsigned long) rem->pone,
			(unsigned long) rem->sone);
		fprintf(f, "  zero        %5lu %5lu\n",
			(unsigned long) rem->pzero,
			(unsigned long)  rem->szero);
	}
	if(rem->ptrail!=0)
	{
		fprintf(f, "  ptrail      %5lu\n",
			(unsigned long) rem->ptrail);
	}
	if(!is_raw(rem))
	{
		if(rem->plead!=0)
		{
			fprintf(f, "  plead       %5lu\n",
				(unsigned long) rem->plead);
		}
		if(has_foot(rem))
		{
			fprintf(f, "  foot        %5lu %5lu\n",
				(unsigned long) rem->pfoot,
				(unsigned long) rem->sfoot);
		}
	}
	if(has_repeat(rem))
	{
		fprintf(f, "  repeat      %5lu %5lu\n",
			(unsigned long) rem->prepeat,
			(unsigned long) rem->srepeat);
	}
	if(!is_raw(rem))
	{
		if(rem->pre_data_bits>0)
		{
			fprintf(f, "  pre_data_bits   %d\n",rem->pre_data_bits);
			fprintf(f, "  pre_data       0x%llX\n",rem->pre_data);
		}
		if(rem->post_data_bits>0)
		{
			fprintf(f, "  post_data_bits  %d\n",rem->post_data_bits);
			fprintf(f, "  post_data      0x%llX\n",rem->post_data);
		}
		if(rem->pre_p!=0 && rem->pre_s!=0)
		{
			fprintf(f, "  pre         %5lu %5lu\n",
				(unsigned long) rem->pre_p,
				(unsigned long) rem->pre_s);
		}
		if(rem->post_p!=0 && rem->post_s!=0)
		{
			fprintf(f, "  post        %5lu %5lu\n",
				(unsigned long) rem->post_p,
				(unsigned long) rem->post_s);
		}
	}
	fprint_remote_gap(f, rem);
	if(has_repeat_gap(rem))
	{
		fprintf(f, "  repeat_gap   %lu\n",
			(unsigned long) rem->repeat_gap);
	}
	if(rem->min_repeat>0)
	{
		fprintf(f, "  min_repeat      %d\n",rem->min_repeat);
	}
	if(!is_raw(rem))
	{
		if(rem->min_code_repeat>0)
		{
			fprintf(f, "  min_code_repeat %u\n",
				rem->min_code_repeat);
		}
		fprintf(f, "  toggle_bit_mask 0x%llX\n",
			rem->toggle_bit_mask);
		if(has_toggle_mask(rem))
		{
			fprintf(f, "  toggle_mask    0x%llX\n",
				rem->toggle_mask);
		}
		if(rem->rc6_mask!=0)
		{
			fprintf(f, "  rc6_mask    0x%llX\n",
				rem->rc6_mask);
		}
		if(has_ignore_mask(rem))
		{
			fprintf(f, "  ignore_mask 0x%llX\n",
				rem->ignore_mask);
		}
		if(is_serial(rem))
		{
			fprintf(f, "  baud            %u\n",rem->baud);
			fprintf(f, "  serial_mode     %uN%u%s\n",
				rem->bits_in_byte,
				rem->stop_bits/2,
				rem->stop_bits%2 ? ".5":"");
		}
	}
	if(rem->freq!=0)
	{
		fprintf(f, "  frequency    %u\n",rem->freq);
	}
	if(rem->duty_cycle!=0)
	{
		fprintf(f, "  duty_cycle   %u\n",rem->duty_cycle);
	}
	fprintf(f,"\n");
}

void fprint_remote_foot(FILE* f, ir_remote const* rem)
{
	fprintf(f, "end remote\n");
}

void fprint_remote_signal_head(FILE* f, ir_remote const* rem)
{
	if(!is_raw(rem))
		fprintf(f, "      begin codes\n");
	else
		fprintf(f, "      begin raw_codes\n\n");
}

void fprint_remote_signal_foot(FILE* f, ir_remote const* rem) noexcept
{
	if(!is_raw(rem))
		fprintf(f, "      end codes\n\n");
	else
		fprintf(f, "      end raw_codes\n\n");
}

void fprint_remote_signal(FILE* f,ir_remote const* rem, ir_ncode const* codes) noexcept
{
	int i,j;

	if(!is_raw(rem))
	{
		char format[30];
		sprintf(format,	"          %%-24s 0x%%0%dllX", (rem->bits+3)/4);
		fprintf(f, format, codes->name->c_str(), codes->code);
		sprintf(format, " 0x%%0%dlX", (rem->bits+3)/4);
		for (ir_code_node* loop = codes->next.get(); loop != nullptr; loop = loop->next.get())
		{
			fprintf(f, format, loop->code);
		}
		
		fprintf(f, "\n");
	}
	else
	{
		fprintf(f, "          name %s\n",codes->name->c_str());
		j=0;
		for(i=0;i<codes->length();i++){
			if (j==0){
				fprintf(f, "          %7lu",
					static_cast<unsigned long>(codes->signals[i]));
			}else if (j<5){
				fprintf(f, " %7lu",
					static_cast<unsigned long>(codes->signals[i]));
			}else{
				fprintf(f, " %7lu\n",
					static_cast<unsigned long>(codes->signals[i]));
				j=-1;
			}
			j++;
		}
		codes++;
		if (j==0)
		{
			fprintf(f,"\n");
		}else
		{
			fprintf(f,"\n\n");
			j=0;
		}
	}
}

void fprint_remote_signals(FILE* f, ir_remote const* rem)
{
	fprint_remote_signal_head(f,rem);
	for (auto& c : rem->codes)
	{
		fprint_remote_signal(f, rem, &c);
	}
	fprint_remote_signal_foot(f,rem);
}


void fprint_remote(FILE* f, ir_remote const* rem, hardware const& hw)
{	
	fprint_comment(f,rem, hw);
	fprint_remote_head(f,rem);
	fprint_remote_signals(f,rem);
	fprint_remote_foot(f,rem);
}
