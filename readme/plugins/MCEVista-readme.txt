MCE Vista 1.3
=============

- Transmit bug fixes.


MCE Vista 1.2
=============

- Fixed some bugs related to transmitting, and blaster selection.
- Receiver is now disabled when transmitting.
- Rewritten the device loading code based on the decompiled code from Windows Media Center.
- Fixed transmitting issues in the 64bit version.
- You can now select which transmitter to transmit on with SET_TRANSMITTERS, the same as LIRC.

MCE Vista 1.1
=============

- Fixed the string name of the device

MCEVista 1.0
============

This is a plugin for the Microsoft MCE (version 2) receiver. It should work with any remote control, not
just the bundled MS remote. This plugin should run on Windows Vista/7 but it will not work on XP. I plan
to write a plugin for XP sometime, but Vista is my main dev OS so it might not be for a while.

Known Issues
============

 - This plugin will require elevated user privilages to run, and or UAC disabled in Vista/7. It wont 
   work with guest accounts. Microsoft's fault.
 - Right click winlirc - run in administrator mode.
 - There is a small issue with the timing of the gap between the keypresses. I have to manually do this
   so it might be prone to error if your CPU is under load. This also means it might not be the most reliable
   of plugins for creating remote configs for other, more accurate receivers. But most of the time it should
   be fine.

Extra Info
==========

This plugin will only work in Windows Vista and 7. It wont work on XP. Use Vista64 for 64bit Vista/7. 

The transmitters have a very limited range, I think they are essentially designed so you can strap them to
whatever device you are trying to control. Transmitting will also fail if you have no blasters attached. I
wrote some code that detects if the blaster is attached, so it will report it has failed if you try to send
with none. They also seem to have a fair bit of cross talk it looks like. Transmitting on one seems to register
a bit on the other, and vice versa.

All the other receivers I've written plugins for have either had some sort of documented API, and
or a provided dll to interface with the device. MCE receivers have nothing ! So I was totally reliant on code
written by other people to interface with these devices. The trouble was some of this code was either broken,
incomplete or just simply wrong, which was a big problem. However messing around I found the dll in Windows 
Media center that contains the code for interfacing with MCE devices, and being .NET it is a peice of cake to 
decompile. A real god send really. So I've rewritten chunks of this project based on code directly written by
MS dev team. So this plugin should be quite stable now.

If you are using the bundled remote, you probably want a config for it, which can be found here:
http://lirc.sourceforge.net/remotes/mceusb/lircd.conf.mceusb


