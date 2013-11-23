AudioInPlugin 1.4
=================

- Fixed a bug that would prevent the device being recognised properly. Device string problem.
- Added an invert audio signal option, for those with funky audio hardware that pulls the signal
  towards the negative voltage instead of positive.
- Added an option to set point at which the signal gets classified as a 1. The midpoint plus a noise
  value. Different hardware seems to need different values. On my htpc after receiving a signal I seem
  to get massive noise for a short period of time, whilst on this pc I don't, and I don't want noise values
  being recognised as input from the receiver ! This option finally allowed repeats to work on my htpc :)

AudioInPlugin 1.3
=================

- Fix for time overflowing and registering as a pulse when it should have been a long space.

AudioInPlugin 1.2
=================

- Minor update to API

AudioInPlugin 1.1
=================

- Changes to ini path

AudioInPlugin 1.0 - by Ian Curtis
=================

This is designed to work with the simplest audio in reciever. http://www.lirc.org/ir-audio.html without the 
extra diode. It should work with the diode, but I've never tested it. I dont think it's really needed. I get
good results at the lowest sampling frequency 11025hz. The left and right options are for whether you have
the input to the left or right input. I only supported this because I accidently wired it to the right input,
and was too lazy to change it to the left. It was easier to fix in software.

The disadvantage over the serial version is, it is not interupt driven. It'll capture audio all the time. But
the overhead is so low it probably won't even register on your CPU. (It doesn't on mine)

The advantage over the serial version is, the audio is all buffered in hardware, making it incensitive to 
whatever else is going on, on your CPU, unlike the serial version which can stop working completely if CPU
usage is 100%.

To troubleshoot this plugin. Firstly check you have plugged your reciever into the LineIn or Microphone in.
Next make sure the volume isn't completely muted, or all the way down to the bottom. As a safe bet you need
at least 50% volume for this to work, although it'll probably work with less. To test you are recieving 
input, either record some sounds or check the volume meter in windows when you press a button. If you are
recieving data but nothing is being decoded, try upping the sampling frequency.

Failing that download Audacity 
http://audacity.sourceforge.net/
And look at what waveform is being captured. You should be able to see if the signal is inverted from the
waveform, and how much noise you are getting.

Websites
========

http://winlirc.sourceforge.net/audioreciever.html
http://www.lirc.org/ir-audio.html