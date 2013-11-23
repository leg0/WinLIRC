Xbox360 1.0
===========

This is a plugin for the wired and wireless Xbox 360 controllers. It works a little differently than the other plugins.
Other plugins will fail to initialise if there is no receiver present. This one won't ! And it also won't complain if 
there are no connected controllers. The reason for this is, often the controller won't be activated when Windows starts.
Also the wireless controller tends to go to sleep after a while. For the wireless Xbox controller, you need to get a special
wireless receiver for the pc. This caught me out first time as I thought the USB charging cable would do the the trick, but
it only provides power to the controller.

This receiver also requires no config files. I built the config into the dll. The names of the buttons are:

	A
	B
	X
	Y
	RSHOULDER
	LSHOULDER
	LTRIGGER
	RTRIGGER

	DPAD_UP
	DPAD_DOWN
	DPAD_LEFT
	DPAD_RIGHT
	START
	BACK
	LTHUMB_PRESS
	RTHUMB_PRESS

	LTHUMB_UP
	LTHUMB_DOWN
	LTHUMB_RIGHT
	LTHUMB_LEFT
	LTHUMB_UPLEFT
	LTHUMB_UPRIGHT
	LTHUMB_DOWNRIGHT
	LTHUMB_DOWNLEFT

	RTHUMB_UP
	RTHUMB_DOWN
	RTHUMB_RIGHT
	RTHUMB_LEFT
	RTHUMB_UPLEFT
	RTHUMB_UPRIGHT
	RTHUMB_DOWNRIGHT
	RTHUMB_DOWNLEFT

Trouble Shooting
===============

You need at least Windows XP with service pack 1 for this to work. 
These drivers should work if you are missing drivers for your receiver:

http://www.microsoft.com/download/en/details.aspx?id=4190 (32bit)
http://www.microsoft.com/download/en/details.aspx?id=8682 (64bit)

