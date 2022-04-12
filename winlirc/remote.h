#pragma once

#include "ir_remote.h"
#include <winlirc/winlirc_api.h>

#include <algorithm>
#include <iterator>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <vector>

struct ir_ncode;
struct ir_code_node;
struct ir_remote;

static inline ir_remote* get_remote_by_name(std::vector<ir_remote>& remotes, const char *name)
{
	auto it = std::find_if(begin(remotes), end(remotes), [&](auto& remote)
		{
			return _stricmp(name, remote.name.c_str()) == 0;
		});
	if (it == end(remotes))
		return nullptr;
	return &*it;
}

static inline auto get_code_by_name(std::vector<ir_ncode>& codes, char const* name)
{
	return std::find_if(begin(codes), end(codes), [&](auto& c)
		{
			return _stricmp(name, c.name->c_str()) == 0;
		});
			
}
