/*      $Id$      */

/****************************************************************************
 ** config_file.c ***********************************************************
 ****************************************************************************
 *
 * config_file.c - parses the config file of lircd
 *
 * Copyright (C) 1998 Pablo d'Angelo <pablo@ag-trek.allgaeu.org>
 *
 */

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
#include "globals.h"

#define LINE_LEN 1024
#define MAX_INCLUDES 10

const char *whitespace = " \t";

static int line;
static int parse_error;

static struct ir_remote * read_config_recursive(FILE *f, const char *name, int depth);

void **init_void_array(struct void_array *ar,size_t chunk_size, size_t item_size)
{
	ar->chunk_size=chunk_size;
	ar->item_size=item_size;
	ar->nr_items=0;
	if(!(ar->ptr=calloc(chunk_size, ar->item_size))){

		parse_error=1;
		return(nullptr);
	}
	return((void **)(ar->ptr));
}

int add_void_array (struct void_array *ar, void * dataptr)
{
	void *ptr;

	if ((ar->nr_items%ar->chunk_size)==(ar->chunk_size)-1){
		/* I hope this works with the right alignment,
		if not we're screwed */
		if (!(ptr=realloc(ar->ptr,ar->item_size*((ar->nr_items)+(ar->chunk_size+1))))){

			parse_error=1;
			return(0);
		}
		ar->ptr=ptr;
	}
	memcpy((char *)(ar->ptr)+(ar->item_size*ar->nr_items), dataptr, ar->item_size);
	ar->nr_items=(ar->nr_items)+1;
	memset((char *)(ar->ptr)+(ar->item_size*ar->nr_items), 0, ar->item_size);
	return(1);
}

inline void *get_void_array(struct void_array *ar)
{
	return(ar->ptr);
}

void *s_malloc(size_t size)
{
	void *ptr;
	if((ptr=malloc(size))==nullptr){
		parse_error=1;
		return(nullptr);
	}
	memset(ptr, 0, size);
	return (ptr);
}

inline char *s_strdup(char * string)
{
	char *ptr;
	if(!(ptr=_strdup(string))){
		parse_error=1;
		return(nullptr);
	}
	return (ptr);
}

/* my very own strtouq */
unsigned __int64 strtouq(const char *val, char **endptr, int base)
{
	while(*val=='\t' || *val==' ') val++;
	if(base==0)
		if(val[0]=='0')
			if(val[1]=='x' || val[1]=='X')
			{
				base=16;
				val+=2;
			}
			else
			{
				val++;
				base=8;
			}
		else
			base=10;
	
	char convert[256];
	for(int i=0;i<255;i++)
	{
		if(i>='0' && i<='9') convert[i]=i-'0';
		else if(i>='a' && i<='f') convert[i]=(i-'a')+10;
		else if(i>='A' && i<='F') convert[i]=(i-'A')+10;
		else convert[i]=-1;
	}

	unsigned __int64 result=0;
	while(*val && convert[*val]!=-1)
	{
		result*=base;
		result+=convert[*val];
		val++;
	}

	*endptr=(char*)val;

	return result;
}

inline ir_code s_strtocode(const char *val)
{
	ir_code code=0;
	char *endptr;

	errno=0;
#       ifdef LONG_IR_CODE
	code=strtouq(val,&endptr,0);
	if((code==(unsigned long long) -1 && errno==ERANGE) ||
		strlen(endptr)!=0 || strlen(val)==0)
	{

		parse_error=1;
		return(0);
	}
#       else
	code=strtoul(val,&endptr,0);
	if(code==ULONG_MAX && errno==ERANGE)
	{
		logprintf(LOG_ERR,"error in configfile line %d:",line);
		logprintf(LOG_ERR,"code is out of range");
		logprintf(LOG_ERR,"try compiling lircd with the LONG_IR_CODE "
			"option");
		parse_error=1;
		return(0);
	}
	else if(strlen(endptr)!=0 || strlen(val)==0)
	{
		logprintf(LOG_ERR,"error in configfile line %d:",line);
		logprintf(LOG_ERR,"\"%s\": must be a valid (unsigned long) "
			"number",val);
		parse_error=1;
		return(0);
	}
#       endif
	return(code);
}

unsigned long s_strtoul(char *val)
{
	unsigned long n;
	char *endptr;

	n=strtoul(val,&endptr,0);
	if(!*val || *endptr)
	{
		parse_error=1;
		return(0);
	}
	return(n);
}

int s_strtoi(char *val)
{
	char *endptr;
	long n;
	int h;

	n=strtol(val,&endptr,0);
	h=(int) n;
	if(!*val || *endptr || n!=((long) h))
	{
		parse_error=1;
		return(0);
	}
	return(h);
}

unsigned int s_strtoui(char *val)
{
	char *endptr;
	unsigned long n;
	unsigned int h;

	n=strtoul(val,&endptr,0);
	h=(unsigned int) n;
	if(!*val || *endptr || n!=((unsigned long) h))
	{
		parse_error=1;
		return(0);
	}
	return(h);
}

lirc_t s_strtolirc_t(char *val)
{
	unsigned long n;
	lirc_t h;
	char *endptr;

	n=strtoul(val,&endptr,0);
	h=(lirc_t) n;
	if(!*val || *endptr || n!=((unsigned long) h))
	{
		parse_error=1;
		return(0);
	}
	if(h < 0)
	{

	}
	return(h);
}

int checkMode(int is_mode, int c_mode, char *error)
{
	if (is_mode!=c_mode)
	{
		parse_error=1;
		return(0);
	}
	return(1);
}

int addSignal(struct void_array *signals, char *val)
{
	lirc_t t;

	t=s_strtolirc_t(val);
	if(parse_error) return(0);
	if(!add_void_array(signals, &t)){
		return(0);
	}
	return(1);
}

struct ir_ncode *defineCode(char *key, char *val, struct ir_ncode *code)
{
	memset(code, 0, sizeof(*code));
	code->name=s_strdup(key);
	code->code=s_strtocode(val);

	return(code);
}

struct ir_code_node *defineNode(struct ir_ncode *code, const char *val)
{
	struct ir_code_node *node;

	node=(ir_code_node*)s_malloc(sizeof(*node));
	if(node==nullptr) return nullptr;

	node->code=s_strtocode(val);
	node->next=nullptr;

	if(code->current==nullptr)
	{
		code->next=node;
		code->current=node;
	}
	else
	{
		code->current->next=node;
		code->current=node;
	}
	return node;
}

int parseFlags(char *val)
{
	struct flaglist *flaglptr;
	int flags=0;
	char *flag,*help;

	flag=help=val;
	while(flag!=nullptr)
	{
		while(*help!='|' && *help!=0) help++;
		if(*help=='|')
		{
			*help=0;help++;
		}
		else
		{
			help=nullptr;
		}

		flaglptr=all_flags;
		while(flaglptr->name!=nullptr){
			if(strcasecmp(flaglptr->name,flag)==0){
				if(flaglptr->flag&IR_PROTOCOL_MASK &&
					flags&IR_PROTOCOL_MASK)
				{
					parse_error=1;
					return(0);
				}
				flags=flags|flaglptr->flag;

				break;
			}
			flaglptr++;
		}
		if(flaglptr->name==nullptr)
		{
			parse_error=1;
			return(0);
		}
		flag=help;
	}

	return(flags);
}

int defineRemote(char * key, char * val, char *val2, struct ir_remote *rem)
{
	if ((strcasecmp("name",key))==0){
		if(rem->name!=nullptr) free(rem->name);
		rem->name=s_strdup(val);
	
		return(1);
	}
	else if ((strcasecmp("bits",key))==0){
		rem->bits=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("flags",key)==0){
		rem->flags|=parseFlags(val);
		return(1);
	}
	else if (strcasecmp("eps",key)==0){
		rem->eps=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("aeps",key)==0){
		rem->aeps=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("plead",key)==0){
		rem->plead=s_strtolirc_t(val);
		return(1);
	}
	else if (strcasecmp("ptrail",key)==0){
		rem->ptrail=s_strtolirc_t(val);
		return(1);
	}
	else if (strcasecmp("pre_data_bits",key)==0){
		rem->pre_data_bits=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("pre_data",key)==0){
		rem->pre_data=s_strtocode(val);
		return(1);
	}
	else if (strcasecmp("post_data_bits",key)==0){
		rem->post_data_bits=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("post_data",key)==0){
		rem->post_data=s_strtocode(val);
		return(1);
	}
	else if (strcasecmp("gap",key)==0){
		if(val2 != nullptr)
		{
			rem->gap2=s_strtoul(val2);
		}
		rem->gap=s_strtoul(val);
		return(val2 != nullptr ? 2:1);
	}
	else if (strcasecmp("repeat_gap",key)==0){
		rem->repeat_gap=s_strtoul(val);
		return(1);
	}
	/* obsolete: use toggle_bit_mask instead */
	else if (strcasecmp("toggle_bit",key)==0){
		rem->toggle_bit = s_strtoi(val);
		return 1;
	}
	else if (strcasecmp("toggle_bit_mask",key)==0){
		rem->toggle_bit_mask = s_strtocode(val);
		return 1;
	}
	else if (strcasecmp("toggle_mask",key)==0){
		rem->toggle_mask=s_strtocode(val);
		return(1);
	}
	else if (strcasecmp("rc6_mask",key)==0){
		rem->rc6_mask=s_strtocode(val);
		return(1);
	}
	else if (strcasecmp("ignore_mask",key)==0){
		rem->ignore_mask=s_strtocode(val);
		return(1);
	}
	/* obsolete name */
	else if (strcasecmp("repeat_bit",key)==0){
		rem->toggle_bit=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("suppress_repeat",key)==0){
		//rem->suppress_repeat=s_strtoi(val);	//TODO support this per remote
		return(1);
	}
	else if (strcasecmp("min_repeat",key)==0){
		rem->min_repeat=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("min_code_repeat",key)==0){
		rem->min_code_repeat=s_strtoi(val);
		return(1);
	}
	else if (strcasecmp("frequency",key)==0){
		rem->freq=s_strtoui(val);
		return(1);
	}
	else if (strcasecmp("duty_cycle",key)==0){
		rem->duty_cycle=s_strtoui(val);
		return(1);
	}
	else if (strcasecmp("baud",key)==0){
		rem->baud=s_strtoui(val);
		return(1);
	}
	else if (strcasecmp("serial_mode",key)==0){
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
		if(strcmp(val+2, "1.5")==0)
		{
			rem->stop_bits=3;
		}
		else
		{
			rem->stop_bits=s_strtoui(val+2)*2;
		}
		return(1);
	}
	else if (val2!=nullptr)
	{
		if (strcasecmp("header",key)==0){
			rem->phead=s_strtolirc_t(val);
			rem->shead=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("three",key)==0){
			rem->pthree=s_strtolirc_t(val);
			rem->sthree=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("two",key)==0){
			rem->ptwo=s_strtolirc_t(val);
			rem->stwo=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("one",key)==0){
			rem->pone=s_strtolirc_t(val);
			rem->sone=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("zero",key)==0){
			rem->pzero=s_strtolirc_t(val);
			rem->szero=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("foot",key)==0){
			rem->pfoot=s_strtolirc_t(val);
			rem->sfoot=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("repeat",key)==0){
			rem->prepeat=s_strtolirc_t(val);
			rem->srepeat=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("pre",key)==0){
			rem->pre_p=s_strtolirc_t(val);
			rem->pre_s=s_strtolirc_t(val2);
			return(2);
		}
		else if (strcasecmp("post",key)==0){
			rem->post_p=s_strtolirc_t(val);
			rem->post_s=s_strtolirc_t(val2);
			return(2);
		}
	}
	if(val2){

	}else{

	}
	parse_error=1;
	return(0);
}

static int sanityChecks(struct ir_remote *rem)
{
	struct ir_ncode *codes;
	struct ir_code_node *node;

	if (!rem->name)
	{
		return 0;
	}
	if(rem->gap == 0)
	{

	}
	if(has_repeat_gap(rem) && is_const(rem))
	{

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
	for(codes = rem->codes; codes->name != nullptr; codes++)
	{
		if((codes->code&gen_mask(rem->bits)) != codes->code)
		{
			codes->code &= gen_mask(rem->bits);
		}
		for(node = codes->next; node != nullptr; node = node->next)
		{
			if((node->code&gen_mask(rem->bits)) != node->code)
			{
				node->code &= gen_mask(rem->bits);
			}
		}
	}

	return 1;
}

struct ir_remote *sort_by_bit_count(struct ir_remote *remotes)
{
	struct ir_remote *top, *rem, *next, *prev, *scan;

	rem = remotes;
	top = nullptr;
	while(rem!=nullptr)
	{
		next = rem->next;

		scan = top;
		prev = nullptr;
		while(scan && bit_count(scan)<=bit_count(rem))
		{
			prev = scan;
			scan = scan->next;
		}
		if(prev)
		{
			prev->next = rem;
		}
		else
		{
			top = rem;
		}
		if(scan)
		{
			rem->next = scan;
		}
		else
		{
			rem->next = nullptr;
		}

		rem = next;
	}

	return top;
}

static const char *lirc_parse_include(char *s)
{
	char *last;
	size_t len;

	len=strlen(s);
	if(len<2)
	{
		return nullptr;
	}
	last = s+len-1;
	while(last >  s && strchr(whitespace, *last) != nullptr)
	{
		last--;
	}
	if(last <= s)
	{
		return nullptr;
	}
	if(*s!='"' && *s!='<')
	{
		return nullptr;
	}
	if(*s=='"' && *last!='"')
	{
		return nullptr;
	}
	else if(*s=='<' && *last!='>')
	{
		return nullptr;
	}
	*last = 0;
	memmove(s, s+1, len-2+1); /* terminating 0 is copied, and
							  maybe more, but we don't care */
	return s;
}

//
// My own win32 version of dirname, bit ugly but C is ugly :p
//
char* dirname(char *path) {

   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];
   char *tmp;
   int stringLength;

   _splitpath( path, drive, dir, fname, ext );

   //
   // convert back slashes to forward for consistency ..
   //

   tmp	= &dir[0];

   while(*tmp) {

	   if(*tmp == '\\') *tmp = '/';
	   tmp++;
   }

   //
   // remove trailing slash
   //
   stringLength = strlen(dir);

   if(stringLength) {
		if(dir[stringLength-1]=='/') dir[stringLength-1] = '\0';
   }

   strcpy(path,dir);
   return path;
}

static const char *lirc_parse_relative(char *dst, size_t dst_size,
									   const char *child, const char *current)
{
	char *dir;
	size_t dirlen;

	if (!current)
		return child;

	/* Not a relative path */
	if (*child == '/')
		return child;

	if(strlen(current) >= dst_size)
	{
		return nullptr;
	}
	strcpy(dst, current);
	dir = dirname(dst);
	dirlen = strlen(dir);
	if(dir != dst)
	{
		memmove(dst, dir, dirlen + 1);
	}

	if(dirlen + 1 + strlen(child) + 1 > dst_size)
	{
		return nullptr;
	}
	strcat(dst, "/");
	strcat(dst, child);

	return dst;
}

struct ir_remote * read_config(FILE *f, const char *name)
{
	return read_config_recursive(f, name, 0);
}

static struct ir_remote * read_config_recursive(FILE *f, const char *name, int depth)
{
	char buf[LINE_LEN+1], *key, *val, *val2;
	int len,argc;
	struct ir_remote *top_rem=nullptr,*rem=nullptr;
	struct void_array codes_list,raw_codes,signals;
	struct ir_ncode raw_code={nullptr,0,0,nullptr};
	struct ir_ncode name_code={nullptr,0,0,nullptr};
	struct ir_ncode *code;
	int mode=ID_none;

	line=0;
	parse_error=0;

	while(fgets(buf,LINE_LEN,f)!=nullptr)
	{
		line++;
		len=strlen(buf);
		if(len==LINE_LEN && buf[len-1]!='\n')
		{
			parse_error=1;
			break;
		}

		if(len>0)
		{
			len--;
			if(buf[len]=='\n') buf[len]=0;
		}
		if(len>0)
		{
			len--;
			if(buf[len]=='\r') buf[len]=0;
		}
		/* ignore comments */
		if(buf[0]=='#'){
			continue;
		}
		key=strtok(buf, whitespace);
		/* ignore empty lines */
		if(key==nullptr) continue;
		val=strtok(nullptr, whitespace);
		if(val!=nullptr){
			val2=strtok(nullptr, whitespace);

			if (strcasecmp("include",key)==0){
				FILE* childFile;
				const char *childName;
				const char *fullPath;
				char result[FILENAME_MAX+1];


				if (depth > MAX_INCLUDES) {
					parse_error=-1;
					break;
				}

				childName = lirc_parse_include(val);
				if (!childName){
					parse_error=-1;
					break;
				}

				fullPath = lirc_parse_relative(result, sizeof(result), childName, name);
				if (!fullPath) {
					parse_error=-1;
					break;
				}

				childFile = fopen(fullPath, "r");
				if (childFile == nullptr){
	
				}
				else{
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

						rem->next=read_config_recursive(childFile, fullPath, depth + 1);
						if(rem->next != (void *) -1 && rem->next != nullptr) {
							rem=rem->next;
						} else {
							rem->next = nullptr;
						}
					}
					fclose(childFile);
					line = save_line;
				}
			}else if (strcasecmp("begin",key)==0){
				if (strcasecmp("codes", val)==0){
					/* init codes mode */

					if (!checkMode(mode, ID_remote,
						"begin codes")) break;
					if (rem->codes){
						parse_error=1;
						break;
					}

					init_void_array(&codes_list,30, sizeof(struct ir_ncode));
					mode=ID_codes;
				}else if(strcasecmp("raw_codes",val)==0){
					/* init raw_codes mode */

					if(!checkMode(mode, ID_remote,
						"begin raw_codes")) break;
					if (rem->codes){

						parse_error=1;
						break;
					}
					set_protocol(rem, RAW_CODES);
					raw_code.code=0;
					init_void_array(&raw_codes,30, sizeof(struct ir_ncode));
					mode=ID_raw_codes;
				}else if(strcasecmp("remote",val)==0){
					/* create new remote */
					
					if(!checkMode(mode, ID_none,
						"begin remote")) break;
					mode=ID_remote;
					if (!top_rem){
						/* create first remote */
						
						rem=top_rem=(ir_remote*)s_malloc(sizeof(struct ir_remote));
					}else{
						/* create new remote */
						
						rem->next=(ir_remote*)s_malloc(sizeof(struct ir_remote));;
						rem=rem->next;
					}
				}else if(mode==ID_codes){
					code=defineCode(key, val, &name_code);
					while(!parse_error && val2!=nullptr)
					{
						struct ir_code_node *node;

						if(val2[0]=='#') break; /* comment */
						node=defineNode(code, val2);
						val2=strtok(nullptr, whitespace);
					}
					code->current=nullptr;
					add_void_array(&codes_list, code);
				}else{
					parse_error=1;
				}
				if(!parse_error && val2!=nullptr)
				{
				}
			}else if (strcasecmp("end",key)==0){

				if (strcasecmp("codes", val)==0){
					/* end Codes mode */
					if (!checkMode(mode, ID_codes,
						"end codes")) break;
					rem->codes=(ir_ncode*)get_void_array(&codes_list);
					mode=ID_remote;     /* switch back */

				}else if(strcasecmp("raw_codes",val)==0){
					/* end raw codes mode */


					if(mode==ID_raw_name){
						raw_code.signals=(lirc_t*)get_void_array(&signals);
						raw_code.length=signals.nr_items;
						if(raw_code.length%2==0)
						{
							parse_error=1;
						}
						if(!add_void_array(&raw_codes, &raw_code))
							break;
						mode=ID_raw_codes;
					}
					if(!checkMode(mode,ID_raw_codes,
						"end raw_codes")) break;
					rem->codes=(ir_ncode *)get_void_array(&raw_codes);
					mode=ID_remote;     /* switch back */
				}else if(strcasecmp("remote",val)==0){
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
					while(!parse_error && val2!=nullptr)
					{
						struct ir_code_node *node;

						if(val2[0]=='#') break; /* comment */
						node=defineNode(code, val2);
						val2=strtok(nullptr, whitespace);
					}
					code->current=nullptr;
					add_void_array(&codes_list, code);
				}else{
					parse_error=1;
				}
				if(!parse_error && val2!=nullptr)
				{

				}
			} else {
				switch (mode){
	case ID_remote:
		argc=defineRemote(key, val, val2, rem);
		if(!parse_error && ((argc==1 && val2!=nullptr) || 
			(argc==2 && val2!=nullptr && strtok(nullptr, whitespace)!=nullptr)))
		{
		}
		break;
	case ID_codes:
		code=defineCode(key, val, &name_code);
		while(!parse_error && val2!=nullptr)
		{
			struct ir_code_node *node;

			if(val2[0]=='#') break; /* comment */
			node=defineNode(code, val2);
			val2=strtok(nullptr, whitespace);
		}
		code->current=nullptr;
		add_void_array(&codes_list, code);
		break;
	case ID_raw_codes:
	case ID_raw_name:
		if(strcasecmp("name",key)==0){
			if(mode==ID_raw_name)
			{
				raw_code.signals=(lirc_t*)get_void_array(&signals);
				raw_code.length=signals.nr_items;
				if(raw_code.length%2==0)
				{
					parse_error=1;
				}
				if(!add_void_array(&raw_codes, &raw_code))
					break;
			}
			if(!(raw_code.name=s_strdup(val))){
				break;
			}
			raw_code.code++;
			init_void_array(&signals,50,sizeof(lirc_t));
			mode=ID_raw_name;
			if(!parse_error && val2!=nullptr)
			{
			}
		}else{
			if(mode==ID_raw_codes)
			{
				parse_error=1;
				break;
			}
			if(!addSignal(&signals, key)) break;
			if(!addSignal(&signals, val)) break;
			if (val2){
				if (!addSignal(&signals, val2)){
					break;
				}
			}
			while ((val=strtok(nullptr, whitespace))){
				if (!addSignal(&signals, val)) break;
			}
		}
		break;
				}
			}
		}else if(mode==ID_raw_name){
			if(!addSignal(&signals, key)){
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
			if(raw_code.name!=nullptr)
			{
				free(raw_code.name);
				if(get_void_array(&signals)!=nullptr)
					free(get_void_array(&signals));
			}
		case ID_raw_codes:
			rem->codes=(ir_ncode *)get_void_array(&raw_codes);
			break;
		case ID_codes:
			rem->codes=(ir_ncode *)get_void_array(&codes_list);
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
		free_config(top_rem);
		if(depth == 0) print_error = 1;
		return(nullptr);
	}
	/* kick reverse flag */
	/* handle RC6 flag to be backwards compatible: previous RC-6
	config files did not set rc6_mask */
	rem=top_rem;
	while(rem!=nullptr)
	{
		if((!is_raw(rem)) && rem->flags&REVERSE)
		{
			struct ir_ncode *codes;

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
			codes=rem->codes;
			while(codes->name!=nullptr)
			{
				codes->code=reverse(codes->code,rem->bits);
				codes++;
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

			if(has_toggle_bit_mask(rem))
			{
			}
			else
			{
				rem->toggle_bit_mask=((ir_code) 1)<<(all_bits-rem->toggle_bit);
			}
			rem->toggle_bit = 0;
		}
		if(has_toggle_bit_mask(rem))
		{
			if(!is_raw(rem) && rem->codes)
			{
				rem->toggle_bit_mask_state = (rem->codes->code & rem->toggle_bit_mask);
				if(rem->toggle_bit_mask_state)
				{
					/* start with state set to 0 for backwards compatibility */
					rem->toggle_bit_mask_state ^= rem->toggle_bit_mask;
				}
			}
		}
		if(is_serial(rem))
		{
			lirc_t base;

			if(rem->baud>0)
			{
				base=1000000/rem->baud;
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
		rem=rem->next;
	}

	top_rem = sort_by_bit_count(top_rem);
#       if defined(DEBUG) && !defined(DAEMONIZE)
	/*fprint_remotes(stderr, top_rem);*/
#       endif
	return (top_rem);
}

void free_config(struct ir_remote *remotes)
{
	struct ir_remote *next;
	struct ir_ncode *codes;

	while(remotes!=nullptr)
	{
		next=remotes->next;

		if(remotes->name!=nullptr) free(remotes->name);
		if(remotes->codes!=nullptr)
		{
			codes=remotes->codes;
			while(codes->name!=nullptr)
			{
				struct ir_code_node *node,*next_node;

				free(codes->name);
				if(codes->signals!=nullptr)
					free(codes->signals);
				node=codes->next;
				while(node)
				{
					next_node=node->next;
					free(node);
					node=next_node;
				}
				codes++;
			}
			free(remotes->codes);
		}
		free(remotes);
		remotes=next;
	}
}
