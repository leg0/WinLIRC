MaxterPlus 1.0
==============

This is a plugin for the Maxter Plus remote. The guys at Elmak kindly donated me a remote so I could
work on this plugin :) The great thing about this remote is, it doesn't need any drivers at all to
work. It just installs itself as a HID device. As such it functions automatically as a keyboard and 
mouse. But these functions can be disabled as desired and we can read the raw data from the device.
So you can leave the windows mouse input from the device, and disable the numbers and use WinLIRC to
remap them inside your program. Quite nifty. The only down side is, you can only use the supplied 
remote with the receiver. But that's not such a deal breaker.

Setup
=====

This plugin requires no config file to work, I just built it into the dll when I made the plugin. 
The list of keys are:

	POWER
	0
	1
	2
	3
	4
	5
	6
	7
	8
	9

	REC_TV
	GUIDE
	LIVE_TV

	BACK
	MORE

	UP
	DOWN
	LEFT
	RIGHT
	OK

	VOL-
	VOL+
	CH/PG-
	CH/PG+

	RECORD
	MCE
	STOP
	MENU
	<<
	||
	>>
	ESC

	|<<
	PLAY
	>>|
	MUTE

	*
	CLEAR
	#

	MOUSE_BUTTON_LEFT
	MOUSE_BUTTON_RIGHT

	MOUSE_UP
	MOUSE_DOWN
	MOUSE_LEFT
	MOUSE_RIGHT

	HELP
	DVD_MENU
	FULLSCREEN
	ENTER

Remote Website
==============

http://maxter.elmak.pl/