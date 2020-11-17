#include "lengths.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

lirc_t calc_signal(lengths* len)
{
	return((lirc_t)(len->sum / len->count));
}

lengths* new_length(lirc_t length)
{
	return new lengths{
		.count = 1,
		.sum = length,
		.upper_bound = length / 100 * 100 + 99,
		.lower_bound = length / 100 * 100,
		.min = length,
		.max = length,
		.next = nullptr
	};
}

int add_length(lengths** first, lirc_t length)
{
	lengths* l, * last;

	if (*first == nullptr)
	{
		*first = new_length(length);
		return 1;
	}
	l = *first;
	while (l != nullptr)
	{
		if (l->lower_bound <= length && length <= l->upper_bound)
		{
			l->count++;
			l->sum += length;
			l->min = std::min(l->min, length);
			l->max = std::max(l->max, length);
			return 1;
		}
		last = l;
		l = l->next;
	}
	last->next = new_length(length);
	return 1;
}

void free_lengths(lengths** firstp)
{
	lengths* first, * next;

	first = *firstp;
	if (first == nullptr) return;
	while (first != nullptr)
	{
		next = first->next;
		delete first;
		first = next;
	}
	*firstp = nullptr;
}

void merge_lengths(lengths* first, lirc_t aeps, unsigned int eps)
{
	lengths* l, * inner, * last;
	unsigned long new_sum;
	int new_count;

	l = first;
	while (l != nullptr)
	{
		last = l;
		inner = l->next;
		while (inner != nullptr)
		{
			new_sum = l->sum + inner->sum;
			new_count = l->count + inner->count;

			if ((l->max <= new_sum / new_count + aeps &&
				l->min + aeps >= new_sum / new_count &&
				inner->max <= new_sum / new_count + aeps &&
				inner->min + aeps >= new_sum / new_count)
				||
				(l->max <= new_sum / new_count * (100 + eps) &&
					l->min >= new_sum / new_count * (100 - eps) &&
					inner->max <= new_sum / new_count * (100 + eps) &&
					inner->min >= new_sum / new_count * (100 - eps)))
			{
				l->sum = new_sum;
				l->count = new_count;
				l->upper_bound = std::max(l->upper_bound, inner->upper_bound);
				l->lower_bound = std::min(l->lower_bound, inner->lower_bound);
				l->min = std::min(l->min, inner->min);
				l->max = std::max(l->max, inner->max);

				last->next = inner->next;
				delete inner;
				inner = last;
			}
			last = inner;
			inner = inner->next;
		}
		l = l->next;
	}
#       ifdef DEBUG
	l = first;
	while (l != nullptr)
	{
		printf("%d x %lu [%lu,%lu]\n", l->count,
			(unsigned long)calc_signal(l),
			(unsigned long)l->min,
			(unsigned long)l->max);
		l = l->next;
	}
#       endif
}

lengths* get_max_length(lengths* first, unsigned int* sump)
{
	unsigned int sum;
	lengths* scan, * max_length;

	if (first == nullptr) return(nullptr);
	max_length = first;
	sum = first->count;

#       ifdef DEBUG
	if (first->count > 0) printf("%u x %lu\n", first->count,
		(unsigned long)calc_signal(first));
#       endif
	scan = first->next;
	while (scan)
	{
		if (scan->count > max_length->count)
		{
			max_length = scan;
		}
		sum += scan->count;
#               ifdef DEBUG
		if (scan->count > 0) printf("%u x %lu\n", scan->count,
			(unsigned long)calc_signal(scan));
#               endif
		scan = scan->next;
	}
	if (sump != nullptr) *sump = sum;
	return max_length;
}

void unlink_length(lengths** first, lengths* remove)
{
	lengths* last, * scan;

	if (remove == *first)
	{
		*first = remove->next;
		remove->next = nullptr;
		return;
	}
	else
	{
		scan = (*first)->next;
		last = *first;
		while (scan)
		{
			if (scan == remove)
			{
				last->next = remove->next;
				remove->next = nullptr;
				return;
			}
			last = scan;
			scan = scan->next;
		}
	}
	printf("unlink_length(): report this bug!\n");
}
