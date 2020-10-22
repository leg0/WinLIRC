IguanaPlugin 1.6
================

- Transmit bug fixes.


IguanaPlugin 1.5
================

- You can select the transmitters now with SET_TRANSMITTERS, the same as LIRC.

IguanaPlugin 1.4
================

- Sent an extra space at the end of a transmission in case the IR transmitter is left on. Might just be
  precautionary but on the serial version this was clearly causing problems. This plugin is for WinLIRC 
  0.8.6b.

IguanaPlugin 1.3
================

- Minor update to API

IguanaPlugin 1.2
=================

- Sending support added !

This plugin is designed to work with WinLIRC 0.8.6a onwards. Simply overwrite the old plugin.

IguanaPlugin 1.1
=================

This is the first USB receiver supported in WinLIRC. The guys at Iguana kindly donated me a receiver for
free to aid development of this plugin :) Simply install the driver software from their website. The 
driver supports multiple receivers. Each extra receiver has a Device ID of x+1 starting from zero. During
development I somehow managed to screw up the receiver and now its on number 3. So bare that in mind if 
the default 0 doesn't work :) I am not sure how you work out what number each receiver is on.

The receiver needs to be line of site really to work. Round the back on my computer doesn't work at all.

Sending support might come later.

Any problems please leave bug reports on sourceforge. If you wish to contact me about this plugin, please email
me at:

i.curtis at gmail dot com

Setup
=====

1. Install http://iguanaworks.net/downloads/windows/iguanaIR-1.0.1.exe
2. Reboot
3. Setup winlirc plugin
4. Make sure receiver has line of site to your remote.



