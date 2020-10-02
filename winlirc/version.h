#ifndef WINLIRC_VERSION

#define WINLIRC_VERSION "0.9.0j"

#define WIDE_(s) L ## s
#define WIDE(s) WIDE_(s)
#define WINLIRC_ID(ver) "WinLIRC " ver " by <jim@jtan.com>, <baily@uiuc.edu> and <i.curtis@gmail.com>"
static wchar_t const idw[] = WIDE(WINLIRC_ID(WIDE(WINLIRC_VERSION)));
static char const id[] = WINLIRC_ID(WINLIRC_VERSION);
#endif

