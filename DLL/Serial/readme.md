Serial Receiver 1.3
===================

- Fixed a critical bug where by the transmitter would be left in the ON state after a signal had been
  sent. Thanks to Rinke for helping me find this bug and testing this build for me. (Still don't have
  any transmitting hardware to test myself). With Rinke's setup this was causing something like a 
  denial of service attack against his IR receiver, stopping his remote from working.
- This build is for WinLIRC 0.8.6b.

Serial Receiver 1.2
===================

- Fixed the string name for the device
- Minor update to API

Serial Receiver 1.1 - by Ian Curtis
===================

- Changes to ini path

Serial Receiver 1.0 - by Ian Curtis
===================

The code is pretty much the same as the old version, but the decoding is now based on LIRC 0.8.6. I've 
only tested the sending part in simulation, so it may not behave entirely correctly. I lack any sending
hardware to test with. If you have any problems, and you only have a serial receiver, download WinLIRC
0.6.5. That was the last stable build with the previous architecture.

Like the previous versions, the serial version is fairly sensitive to timing. So if your CPU usage is 
high or maxed out, the timing might not work correctly. You can try bumping WinLIRC to have real time
CPU usage if that helps at all.

Extra Info
==========

This receiver WILL NOT WORK with a USB -> Serial converter.
You will get signals, but the timing won't be accurate to do anything useful with. You are welcome to
try though. A lot of modern motherboards actually lack com port these days, which is a real shame. But
all is not lost as most motherboards still have the pins on the motherboard for serial ports, you just
need something to plug into them, or failing that wire directly to the motherboard.
http://htpc-hell.zxq.net/?p=61 like this guy did. 
http://blog.mymediasystem.net/uncategorized/setting-up-a-serial-lirc-receiver-at-com2/ or this guy.

Websites
========

http://www.lirc.org/images/schematics.gif