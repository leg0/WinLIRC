WinLIRC 0.9.0g
==============

- Decoding bug fixes for the audio plugin.
- Changed the MAX_SIGNALS in irrecord to support remotes with crazy long codes (air conditioners)
- Plugins now have 1 second to re-init after sleep. Should stop usb failing after re-awakening from sleep.
- Two parting transmission code is (hopefully) working.
- The IRToy plugin now only shows valid com ports on the system.
- Geniatech T200, T220, T220A are now supported by the Geniatech plugin.

WinLIRC 0.9.0f
==============

- Plugins that don't support recording of remotes now have the options blanked out in WinLIRC.
- Implemented the rest of the LIST LIRC protocol.
- Fixed invalid commands reporting success.
- Fixed the server breaking if the max number of clients connected.
- 1 new plugin for Hauppage TV cards ( thanks Philipp Borsutzki )

Win/LIRC could really do with a central database of remotes. 
Something web 2.0 where people can upload and rate configs.
If anyone has the skills or wants to do this please contact me at the email address at the bottom !
I can make something to validate config files to stop people uploading spam.

WinLIRC 0.9.0e
==============

- A fix to stop plugins failing to load when the program is started from the registry or by other tools that
  mess with the current directory of the program. ( Thanks leg0 )
- Fixed a crash when the setup dialog was closed. ( Thanks leg0 )
- A fix for the IRMan plugin. The device requires the RTS and DTS pins to be on to work, and this was
  previously missing.
- Removed unneeeded putting back into sample mode during transmit for the IRToy.

WinLIRC 0.9.0d     - 16/4/2012
==============

- 3 new plugins for TBS and Prof compatible DVB tuners - ( Thanks Crazycat )

WinLIRC 0.9.0d
==============

- Rewritten the server part a bit to properly support transmit repeats via TCP
- 2 new plugins for a range of satellite tv cards based upon Skystar and Geniatech chipsets. ( Thanks Crazycat )
- Brand new icon ! (Big thanks to the guys at Zoomplayer for this - http://www.inmatrix.com ) 

TODO
---------

- Transmitting support for MCE XP plugin.

WinLIRC 0.9.0c
==============

- New plugin for the CommandIR hardware (thanks Matthew)
- More transmit bug fixes !

WinLIRC 0.9.0b
=============

- 2 new plugins for TV cards - thanks crazycat :)
- Increased the size of the sending buffer for transmitting, long protocols like RC6 with enough
  repeats were coming out larger than our transmit buffer size. Effects MCE and IRToy plugins.
- Added an option to automatically create configs (using irrecord) from the GUI. No more command line
  needed.

WinLIRC 0.9.0a
==============

- New Xbox 360 controller plugin.
- Totally rewritten Streamzap plugin.
- Bug fixes to all MCE receiver plugins.
- Added support for SET_TRANSMITTERS in the WinLIRC server and to the MCE and Iguana plugins.

WinLIRC 0.9.0
=============

- MCE plugin for Vista/7 64, large changes to audio receiver plugin, fixes to IR Toy USB plugin. 
- New beholder tv tuner plugin.
- Various other fixes
- New option to disable system tray.
- RawCodes example client which shows the raw remote messages coming from the WinLIRC server.
- Special thanks to Price for supporting me with this release, and PETY3bI for the Beholder plugin.

WinLIRC needs your help. We could do with a better website, and a Web 2.0 user uploadable database
of remote control configs. Can you help ? :)

WinLIRC 0.8.7a - 15/05/2011
==============

- Bundled some updated plugins. Should have a working MCE plugin for XP.

WinLIRC 0.8.7a
==============

- Fixed some crash vulnerabilities in the WinLIRC server. The vulnerabilities exist in all previous versions of
  WinLIRC.
- Transmitting inside the program worked fine, but externally from command line or by TCP was hopelessly broken.
  The sending string wasn't null terminated so it was basically fluke if it worked or crashed. Again this problem
  was in all previous versions.
- Bundled two new plugins, one for the Firefly mini remote, and the other for the Microsoft MCE receiver.
- Fixed a few handle leaks in some of the plugins.

WinLIRC 0.8.7
=============

- Changed the default for the server to only allow localhost connections.
- Updates to IRRecord
- Bundled all the updated plugins. Ie the Serial plugin in the previous version had a critical bug in it where
  the transmitter was left on. Updated plugins are always available in the files section. Always check to make
  sure you have the latest version.
- Bundled some of the other tools with WinLIRC such as IRGraph.

WinLIRC 0.8.6b
==============

- Much better error handling with regards to config files. Previously there were no warnings if the config
  file path was invalid, or there was a problem with the config file.
- Made improvements to how repeats are handled. You can now skip the first few repeats if your remote sends
  too many. For me this enables menu navigation, whilst still being able to get repeats if I hold down
  the remote longer. Technically you can put many remotes in a single config file, so these settings are 
  global. But I think most people will only use 1 remote at a time.
- Bundled the updated Iguana plugin. Now with sending support. Check the files section on sourceforge for
  updates for your receiver plugin.
- I've changed the hardware export API slightly to add an extra function for other receivers. This change has
  affected all plugins. But it's only a minor change.
- Updated IRRecord accordingly.
- Please submit bug reports to the sourceforge webpage, if you have problems. I can't help when you just leave
  negative feedback when things don't work.

WinLIRC 0.8.6a 
==============

- Fixed a file handle leak
- Changed the program so it doesn't need a config file. Potentially some receiver types might not need them,
  as they might only support 1 remote anyway. Ie receiver/remote info built into plugin dll.
- Changed the CurrentDirectory to the plugins folder. This change effected all the plugins. So don't use older
  plugins with this build as they will look for the ini file in the wrong folder. Otherwise the API is the same.
- Added support for a USB receiver.


WinLIRC 0.8.6 (based on LIRC 0.8.6) - by Ian Curtis
=============

I've made some quite large changes to WinLIRC. The original program was hard coded to basically use serial 
recievers. While this was fine for many years, a lot of newer pcs don't have serial ports ! And that 
presents a big problem. I wanted to build my own simple reciever and support it in the original version of
WinLIRC. The audio input reciever would essentially share the same code base as the serial version, as it
works with raw timing information. But I came to the conclusion that it would just be too messy to try and 
add support for a second reciever, so I redesigned the whole program. 

WinLIRC now only acts a server/GUI, and loads config files. All the decoding is now from from within a DLL
file. I created a simple C API for this.


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

	#ifdef __cplusplus
	}
	#endif

	#endif

If you want to add support for a new reciever, just export those functions from a DLL. sendIR and loadSetupGui
can be stubbed if needed. Most of the decode code can be taken from the LIRC project. Copy and paste directly.

The init function should create/destroy a thead if needed. In my audio example it doesn't need this, as the 
Windows API automatically creates a thread for capturing audio, and destroys it again when I stop. The 
decodeIR function runs in a seperate thread. It runs in the daemon thread from WinLIRC itself, and should be
exited when exitEvent is triggered.

I decided to write all the settings to the INI file, as the program has no uninstaller. Just makes things
simpler. The registery is no longer used. If you write a plugin, feel free to put the settings in the same 
file.

Although not required, the DLLs also export a hardware struct. This struct will contain information that the
IRRecord program, when it's done will eventually use. This is slightly different to the standard LIRC one,
but near enough. The idea is, the IRRecord program will just be able to load whichever dll you have for your 
reciever and you will be able to record input with it.

If you have any problems with this, feel free to email me, or leave a bug report on sourceforge. If you have
problems with the serial version, just roll back to the older version which is 0.6.5, since that was the last
stable build with the old architecture.

If you wish to contact me about this project, feel free to mail me at:
----------------------
i.curtis at gmail.com.
----------------------