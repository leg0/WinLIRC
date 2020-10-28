#pragma once

#include "ir_remote.h"
#include <winlirc/winlirc_api.h>

#include <sys/types.h>
#include <string.h>
#include <math.h>

#define strcasecmp _stricmp

struct ir_ncode;
struct ir_code_node;
struct ir_remote;

static inline ir_remote* get_remote_by_name(ir_remote* remotes, const char *name)
{
	while (remotes!=nullptr&& _stricmp(name,remotes->name)) {
		remotes = remotes->next;
	}

	return remotes;
}

static inline ir_ncode* get_code_by_name(ir_ncode*codes, const char *name)
{
	while (codes->name!=nullptr&& _stricmp(name,codes->name)) {
		codes++;
	}

	return codes;
}
