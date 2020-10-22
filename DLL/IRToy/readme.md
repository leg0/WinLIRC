IRToy 1.5
=========

- Removed unneeeded putting back into sample mode during transmit.

IRToy 1.4
=========

- Transmit bug fixes.

IRToy 1.3
=========

- Rewritten transmitting code based on new firmware. With the old firmware you could crash the IR Toy by
  sending repeated transmissions too fast. The new code should be more reliable as the IR Toy now has a
  double buffered transmission buffer.
- You NEED the latest firmware to use this plugin. If you really want to use older firmwares, download,
  an older dll from the previous winlirc releases.

IRToy 1.2
=========
- Fix for a recording problem with irrecord with some remotes.

IRToy 1.1
=========

- Support for more com ports
- Sending support

You must have firmware 1.07 or later to run this version. (Actually 1.05 might still work without the 
sending support).


IRToy 1.0 - by Ian Curtis
=========

This is plugin for the Dangerous Prototypes IR Toy USB infrared receiver. The guys at Dangerous Prototypes
kindly donated me a receiver so I could build this plugin. (I love receiving free hardware :D) The IR Toy
is an IRMan compatable receiver. I have no code (yet) to support IRMan receivers, and they seem to have
certain limitations. IRMan config files can only be used with IRMan receivers, and this sucks really. I 
suggested they added mode2 format to their receiver (raw timing mode), which they did specifically for
this plugin, which is really quite awesome. The hardware is all open source and programmable by you.

In order to get this to work, you must be running version 1.05 of the firmware (or later). Instructions to
upgrade the firmware can be found here:
http://dangerousprototypes.com/docs/USB_IR_Toy_firmware_update

The firmware itself can be found here:
http://code.google.com/p/dangerous-prototypes-open-hardware/downloads/list

The only setting needed in WinLIRC to make this work is the com port. Look in device manager in Windows
to see which com port the device has assigned itself to.

Troubleshooting
===============

- Make sure the receiving is running the latest firmware.
- You can see what com port the IR Toy is registered as in device manager in the control panel.

Receiver Website
================

http://dangerousprototypes.com/2010/01/29/prototype-usb-infrared-remote-control-receivertransmitter/
