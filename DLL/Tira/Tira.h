#ifndef TIRA_H
#define TIRA_H

//
// TIRA API
//
#define TI_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

TI_API int	init		(HANDLE exitEvent);
TI_API void	deinit		();
TI_API int	hasGui		();
TI_API void	loadSetupGui();
TI_API int	sendIR		(struct ir_remote *remotes, struct ir_ncode *code, int repeats);
TI_API int	decodeIR	(struct ir_remote *remotes, char *out);

//
// This function will be for the IR-record port, well that's the plan anyway
// It's not needed by the main app
//
TI_API struct hardware* getHardware();

#ifdef __cplusplus
}
#endif

#endif