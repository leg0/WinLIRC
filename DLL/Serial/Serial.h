#ifndef WINLIRCAUDIOIN_H
#define WINLIRCAUDIOIN_H

//
// Serial Input API
//
#define SI_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	
SI_API int	init		(HANDLE exitEvent);
SI_API void	deinit		();
SI_API int	hasGui		();
SI_API void	loadSetupGui();
SI_API int	sendIR		(struct ir_remote *remotes, struct ir_ncode *code, int repeats);
SI_API int	decodeIR	(struct ir_remote *remotes, char *out);

//
// This function will be for the IR-record port, well that's the plan anyway
// It's not needed by the main app
//
SI_API struct hardware* getHardware();

#ifdef __cplusplus
}
#endif

#endif