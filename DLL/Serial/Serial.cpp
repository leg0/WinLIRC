#include "Globals.h"
#include <stdio.h>
#include "LIRCDefines.h"
#include "Serial.h"
#include "SerialDialog.h"
#include "irdriver.h"
#include "Decode.h"
#include "hardware.h"
#include "Send.h"


SI_API int init(HANDLE exitEvent) {

	threadExitEvent	= exitEvent;

	initHardwareStruct();
	init_rec_buffer();

	irDriver = new CIRDriver();
	if(irDriver->InitPort()) return 1;

	return 0;
}

SI_API void deinit() {

	threadExitEvent = NULL;	//this one is created outside the DLL

	if(irDriver) {
		delete irDriver;
		irDriver = NULL;
	}
}

SI_API int hasGui() {

	return TRUE;
}

SI_API void loadSetupGui() {

	SerialDialog serialDialog;
	serialDialog.DoModal();
}

SI_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	//
	// This function is totally untested since i have no hardware to test it
	// By some miracle it might work
	//

	send(code,remotes,repeats);

	return 1;
}

SI_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(irDriver) {
		irDriver->waitTillDataIsReady(0);
	}

	if(decodeCommand(remotes, out)) {
		return 1;
	}
	
	return 0;
}

SI_API struct hardware* getHardware() {

	initHardwareStruct();	//make sure values are setup

	return &hw;
}