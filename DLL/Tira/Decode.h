#ifndef DECODE_H
#define DECODE_H

int WINAPI tiraCallbackFunction(const char * eventstring);
bool decodeCommand(struct ir_remote *remotes, char *out);

#endif