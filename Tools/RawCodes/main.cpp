#include <Windows.h>
#include <tchar.h>
#include "irdriver.h"
#include "Settings.h"
#include <winlirc/PluginApi.h>
#include <conio.h>

void main()
{
	//==========================
	Settings	settings;
	TCHAR		pluginPath[128];
	CIRDriver	irDriver;
	//==========================

	hardware const* hw = NULL;

	SetCurrentDirectory(_T(".\\plugins\\"));

	if(settings.getPluginName(pluginPath)) {

		if(irDriver.loadPlugin(pluginPath)) {

			hw = irDriver.getHardware();

			if(hw==NULL ) {
				printf("The plugin doesn't export the required functions.");
				return;
			}
		}
		else {
			printf("Loading plugin failed.");
			return;
		}
	}
	else {
		printf("No valid plugins found.");
		return;
	}

	if(hw->rec_mode!=LIRC_MODE_MODE2 && hw->rec_mode!=LIRC_MODE_LIRCCODE) {
		printf("The plugin doesn't export any raw data.");
		return;
	}

	if(!irDriver.init()) {
		printf("Intialising plugin failed.");
		return;
	}

	if(hw->rec_mode==LIRC_MODE_MODE2) {

		while(1) {

			//==========
			lirc_t data;
			//==========

			while(hw->data_ready()) {

				data = hw->readdata(0);

				if(data&PULSE_BIT) {
					printf("PULSE %i\n",data&PULSE_MASK);
				}
				else {
					printf("SPACE %i\n",data&PULSE_MASK);
				}
			}
			if(_kbhit()) break;	//user pressed a key

			Sleep(10);
		}

	}
	else {

		while(1) {

			//===========
			ir_code code;
			//===========

			while(hw->data_ready()) {

				code = hw->get_ir_code();

				printf("%I64x\n",code);

			}
			if(_kbhit()) break;	//user pressed a key

			Sleep(10);
		}

	}

	irDriver.deinit();
}