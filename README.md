# WinLIRC

![Build Release](https://github.com/leg0/WinLIRC/workflows/Build%20Release/badge.svg?branch=master)

Forked from http://winlirc.sourceforge.net/

# Prerequisites

  * [Visual Studio 2022](https://visualstudio.microsoft.com/) (any edition). Make sure you install MFC
  * [CMake](https://cmake.org/download/).

# Building

You need Visual Studio, any edition will do. If that's installed, just run `quickstart.bat x86` or `quickstart.bat x64`
to get all the depepndencies and initialize Visual Studio solution. You can build winlirc using cmake on command line
or you can open the generated .sln file and build it with Visual Studio. Here's an eample how to build and install
WinLIRC with CMake on command line:

    c:\winlirc> bootstrap.bat x64
    c:\winlirc> cd _build-x64
    c:\winlirc\_build-x64> cmake --build . --config RelWithDebInfo
    c:\winlirc\_build-x64> cmake --install . --prefix %userprofile%\winlirc --config RelWithDebInfo


# Share a config file

Have a config file you want to share? Please go to https://github.com/leg0/winlirc-configs and create a pull request.
