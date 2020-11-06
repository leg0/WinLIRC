#pragma once

#include "constants.h"
#include <stdio.h>
#include <sys/types.h>

struct flaglist
{
	char const *name;
	int flag;
};

static constexpr flaglist all_flags[]=
{
	{"RAW_CODES",       RAW_CODES},
	{"RC5",             RC5},
	{"SHIFT_ENC",       SHIFT_ENC}, /* obsolete */
	{"RC6",             RC6},
	{"RCMM",            RCMM},
	{"SPACE_ENC",       SPACE_ENC},
	{"SPACE_FIRST",     SPACE_FIRST},
    {"GOLDSTAR",        GOLDSTAR},
	{"GRUNDIG",         GRUNDIG},
	{"BO",              BO},
	{"SERIAL",          SERIAL},
	{"XMP",             XMP},
	
	{"REVERSE",         REVERSE},
	{"NO_HEAD_REP",     NO_HEAD_REP},
    {"NO_FOOT_REP",     NO_FOOT_REP},
	{"CONST_LENGTH",    CONST_LENGTH}, /* remember to adapt warning message when changing this */
    {"REPEAT_HEADER",   REPEAT_HEADER},
};

/*
  config stuff
*/

enum directive {ID_none,ID_remote,ID_codes,ID_raw_codes,ID_raw_name};

/* some safer functions */
void * s_malloc(size_t size);

struct ir_remote;

int checkMode(int is_mode, int c_mode, char *error);
ir_remote *read_config(FILE *f, const char *name);
void free_config(ir_remote *remotes);
