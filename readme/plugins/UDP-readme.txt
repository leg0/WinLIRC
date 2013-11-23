UDP Plugin 1.0
==============

This is a LIRC compatable UDP plugin. Devices that support sending UDP on Linux should be able to send data
to WinLIRC and have it decoded successfully. It listens for data on port 8765. It only works with mode2 data,
which is essentially raw timing codes. This plugin is for WinLIRC 0.8.6b, it might work with older ones, but
I won't support that.

Developer Notes
===============

Sending UDP data should be fairly easy in most languages, and for some people might be an easier option than
writing their own WinLIRC plugin. On the plus side you can support both WinLIRC and LIRC with the same code.
The UDP format is a little bit strange, I am not sure why the developer chose this particular data layout.

 * Received UDP packets consist of some number of LE 16-bit integers.
 * The high bit signifies whether the received signal was high or low;
 * the low 15 bits specify the number of 1/16384-second intervals the
 * signal lasted.

So,

Bit  16    - Is this data pulse or space
Bits 15-1  - Represents the time in 1/16384-second intervals

What caught me out was the fact that when bit 16 is 1, this represents a space, and when it's zero, this
represents a pulse. The inverse of what LIRC uses. I've written a simple function to change between LIRC
timing data and the format this plugin uses.

LIRC timing data is laid out like this:



typedef int lirc_t;

#define PULSE_BIT  0x01000000
#define PULSE_MASK 0x00FFFFFF

A pulse is marked with the bit value 0x01000000.
The last 24 bits are the time of the pulse / space in microseconds.

To convert between that layout and the UDP layout I use this function.

unsigned short lircValueToUDP(lirc_t length) {

    //==========================
    int			space;
    unsigned short	returnValue;
    //==========================

    if(length & PULSE_BIT) {
        space = 0;
    }
    else {
        space = 1;
    }

    length	= length & PULSE_MASK;
    returnValue = (length<<8) / 15625;				// (length * 16384) / 1000000;

    if(returnValue > 0x7FFF) returnValue = 0x7FFF;
    if(space) returnValue |= 0x8000;
    
    return returnValue;
}