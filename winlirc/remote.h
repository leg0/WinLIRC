#pragma once

#include "ir_remote.h"
#include <winlirc/winlirc_api.h>

#include <algorithm>
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

static inline ir_ncode* get_code_by_name(void_array<ir_ncode>& codes, const char *name)
{
	return std::find_if(begin(codes), end(codes), [=](ir_ncode& code) {
		return _stricmp(name, code.name) == 0;
	});
}
