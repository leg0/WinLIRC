#pragma once
#include <winlirc/PluginApi.h>

struct lengths
{
	unsigned int count;
	lirc_t sum, upper_bound, lower_bound, min, max;
	lengths* next;
};

struct lengths* new_length(lirc_t length);
int add_length(struct lengths** first, lirc_t length);
void free_lengths(struct lengths** firstp);
void merge_lengths(struct lengths* first, lirc_t aeps, unsigned int eps);
void unlink_length(struct lengths** first, struct lengths* remove);
struct lengths* get_max_length(struct lengths* first, unsigned int* sump);
lirc_t calc_signal(struct lengths* len);
