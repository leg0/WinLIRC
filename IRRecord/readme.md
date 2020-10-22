IRRemote
========

Use this program to generate configs for your remote. This program is also useful to check your receiver is
working properly as it doesn't rely on any pre-existing config files to work. IRRecord loads the plugins in the
same way WinLIRC does, but it has no GUI, so the plugins must be setup correctly from inside of WinLIRC in 
order to work in IRRecord. The current directory is set to the plugins folder, so this explains the file paths,
needed by the program to work correctly.

Usage
=====

IRRecord.exe -d AudioCapture.dll ..\config.cfg

OhNo
====

If for whatever reason it doesn't work. Try downloading your config file from
http://lirc.sourceforge.net/remotes/

Other Info
==========

http://www.lirc.org/html/irrecord.html
