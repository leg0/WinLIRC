Streamzap 1.1
=============

I've totally rewritten this plugin ! Originally I wrote it using the irdata.dll file provided to
me by the kind guys at streamzap, and it worked fine for their remote. I wrote a special decoder
to work with their remote control, and it works really nicely to be honest. I also wrote a different
plugin the StreamzapAlternate driver which acts a bit more like a standard LIRC receiver, ie needs 
config files. The problem was though, even though the receiver is capable of receiving raw timing 
information there was a bug in their irdata.dll which meant some of the signals were getting lost.
Which meant most remote controls didn't work properly, not even their own without the special decoder
I wrote. I pointed this out to them, but sadly they never got around to fixing it. However with some clever
guess work, registry hacking, and decompiling of their irdata.dll I worked out how to talk directly
to the hardware without their API ! This means the streamzap receiver can be used with all remotes !
Well, nearly all. It works happily with all RC5, some other protocols trip up because the sampling resolution
of the device is quite low, 256us. This leaves us with a maximum possible error of 128us, if we pick the mid
point.

If you are having trouble with other remotes, up the error values in the config files. For example:

  eps                  50
  aeps                150

aeps is the absolute error. The default is 100, if our max error is 128 you can see the problem.
eps is the relative tolerence, that needs to be upped from the default also !

By upping the error values on problem remotes, basically I can get all the remotes on my desk to work,
even RC6/MCE, sony, most others. :) You may need to use a template file with higher error values for 
irrecord to work.

Since the config file is not built in anymore, for the streamzap remote you'll need to provide one,
which you can find here:

http://lirc.sourceforge.net/remotes/streamzap/

Enjoy !


Troubleshooting
===============

- Close the Streamzap software ! Only one connection to the remote is allowed. If their program is
  open the plugin can't initialise.
- 64bit drivers -> http://www.streamzap.com/tmp/x64.zip
- http://lirc.sourceforge.net/remotes/streamzap/


Remote website
==============

http://www.streamzap.com


Contact
=======

If you want to contact me about this plugin mail me at i.curtis at gmail dot com. Any problems
please leave bug reports on sourceforge so I can help resolve them.


