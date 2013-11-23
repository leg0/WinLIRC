FireFlyMini 1.1
===============

- Fix for firefly mini not being detected (and thus not working) on Windows XP. 7 and Vista were fine.
  Thanks to Drew for finding this one.

FireFlyMini 1.0
===============

This is a plugin for the firefly mini remote. The remote installs itself as a HID device, so you 
shouldn't have to install any drivers for this.

Some of buttons automatically send input to the active window. Buttons 0-9 do this, as well as the
arrow keys, the mute button, vol+/- and the stop key. The rest of the keys have no effect on the 
active window. All buttons give output to WInLIRC. Due to the nature of this device, I had to write
2 different methods to read the keys. The keys that effect the current window allow repeats, where as
the others don't.

Installation and setup
======================

Simply copy the dll to the WinLIRC plugins folder and select in WinLIRC. The plugin requires no 
config to work, since I built the values into the plugin itself. The keys you need to know are:

	CLOSE
	MUTE
	VOL+
	VOL-
	CH+
	LAST
	EXIT
	OPTION
	FIREFLY
	MENU
	REC
	GUIDE
	STOP
	PREV
	PLAY
	NEXT
	REW
	PAUSE
	FWD
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
	LEFT
	RIGHT
	UP
	DOWN
	OK

Remote Website
==============

http://www.snapstream.com/products/fireflymini/


