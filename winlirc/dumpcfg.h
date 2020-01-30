/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1998 Pablo d'Angelo (pablo@ag-trek.allgaeu.org)
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#pragma once

#include "globals.h"

#include <stdio.h>
#include <sys/types.h>

#include "remote.h"

struct ir_remote;
struct ir_ncode;

void fprint_comment(FILE* f, ir_remote const* rem);
void fprint_flags(FILE* f, int flags);
void fprint_remotes(FILE* f, ir_remote const* all);
void fprint_remote_head(FILE* f, ir_remote const* rem);
void fprint_remote_foot(FILE* f, ir_remote const* rem);
void fprint_remote_signal_head(FILE* f, ir_remote const* rem);
void fprint_remote_signal_foot(FILE* f, ir_remote const* rem);
void fprint_remote_signal(FILE* f, ir_remote const* rem, ir_ncode* codes);
void fprint_remote_signals(FILE* f, ir_remote const* rem);
void fprint_remote(FILE* f, ir_remote const* rem);
