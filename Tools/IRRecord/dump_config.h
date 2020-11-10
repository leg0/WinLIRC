/*      $Id: dump_config.h,v 5.2 2007/07/29 18:20:06 lirc Exp $      */

/****************************************************************************
 ** dump_config.h ***********************************************************
 ****************************************************************************
 *
 * dump_config.h - dumps data structures into file
 *
 * Copyright (C) 1998 Pablo d'Angelo <pablo@ag-trek.allgaeu.org>
 *
 */ 

#ifndef  _DUMP_CONFIG_H
#define  _DUMP_CONFIG_H

#include <stdio.h>
#include <sys/types.h>

struct ir_remote;
struct ir_ncode;

void fprint_comment(FILE* f, ir_remote const* rem) noexcept;
void fprint_flags(FILE* f, int flags) noexcept;
void fprint_remotes(FILE* f, ir_remote const* all);
void fprint_remote_gap(FILE* f, ir_remote const* rem) noexcept;
void fprint_remote_head(FILE* f, ir_remote const* rem);
void fprint_remote_foot(FILE* f, ir_remote const* rem);
void fprint_remote_signal_head(FILE* f, ir_remote const* rem);
void fprint_remote_signal_foot(FILE* f, ir_remote const* rem) noexcept;
void fprint_remote_signal(FILE* f, ir_remote const*rem, ir_ncode const* codes) noexcept;
void fprint_remote_signals(FILE* f, ir_remote const* rem);
void fprint_remote(FILE* f, ir_remote const* rem);

#endif
