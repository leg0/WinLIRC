#pragma once

#include <stdio.h>
#include <sys/types.h>

struct hardware;
struct ir_remote;
struct ir_ncode;

void fprint_comment(FILE* f, ir_remote const* rem, hardware const& hw) noexcept;
void fprint_flags(FILE* f, int flags) noexcept;
void fprint_remotes(FILE* f, ir_remote const* all, hardware const& hw);
void fprint_remote_gap(FILE* f, ir_remote const* rem) noexcept;
void fprint_remote_head(FILE* f, ir_remote const* rem);
void fprint_remote_foot(FILE* f, ir_remote const* rem);
void fprint_remote_signal_head(FILE* f, ir_remote const* rem);
void fprint_remote_signal_foot(FILE* f, ir_remote const* rem) noexcept;
void fprint_remote_signal(FILE* f, ir_remote const*rem, ir_ncode const* codes) noexcept;
void fprint_remote_signals(FILE* f, ir_remote const* rem);
void fprint_remote(FILE* f, ir_remote const* rem, hardware const& hw);
