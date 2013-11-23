MCEXP 1.1
=========

- Now compatable with generation 1 MCE receivers.
- Rewritten the device loading code based on the decompiled code from Windows Media Center.

MCEXP 1.0
=========

This is a plugin for the Microsoft MCE (version 2) receiver. It should work with any remote control, not
just the bundled MS remote. This plugin has only had limited testing, so if issues arise please contact me
at i.curtis at gmail.com or through sourceforge.

Known Issues
============

 - There is a small issue with the timing of the gap between the keypresses. I have to manually do this
   so it might be prone to error if your CPU is under load. This also means it might not be the most reliable
   of plugins for creating remote configs for other, more accurate receivers. But most of the time it should
   be fine.
 - No sending support yet.

Extra Info
==========

This plugin will only work with WinXP 32bit.
If you are using the bundled remote, you probably want a config for it, which can be found here:
http://lirc.sourceforge.net/remotes/mceusb/lircd.conf.mceusb
