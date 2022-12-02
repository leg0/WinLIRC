# WinLIRC

![Build Release](https://github.com/leg0/WinLIRC/workflows/Build%20Release/badge.svg?branch=master)

Forked from http://winlirc.sourceforge.net/

# Prerequisites

## For building in Visual Studio 
  * [Visual Studio 2022](https://visualstudio.microsoft.com/) (any edition). Make sure you install MFC
  * [CMake](https://cmake.org/download/). Don't really need to install it separately - it comes with Visual Studio.

## For building in Visual Studio Code
  * [Visual Studio Code](https://code.visualstudio.com/)
  * [Visual Studio or Visual Studio build tools](https://visualstudio.microsoft.com/)
  * [CMake](https://cmake.org/download/). 
  * [CMake Tools extension for vscode](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)

# Building

## Visual Studio

There are two options when building with Visual Studio - using the CMake support built into Visual Studio, or
by generating the Visual Studio solution, and using that.

### With Generated Visual Studio solution
You need Visual Studio, any edition will do. If that's installed, just run `quickstart.bat x86` or `quickstart.bat x64`
to get all the depepndencies and initialize Visual Studio solution. You can build winlirc using cmake on command line
or you can open the generated .sln file and build it with Visual Studio. Here's an eample how to build and install
WinLIRC with CMake on command line:

    c:\winlirc> bootstrap.bat x64
    c:\winlirc> cd _build-x64
    c:\winlirc\_build-x64> cmake --build . --config RelWithDebInfo
    c:\winlirc\_build-x64> cmake --install . --prefix %userprofile%\winlirc --config RelWithDebInfo

### With built-in CMake support
Open the root folder of the repo in Visual Studio. Let Visual Studio configure the project. When
Visual Studio finishes generating the build system, then just start building.

# Share a config file

Have a config file you want to share? Please go to https://github.com/leg0/winlirc-configs and create a pull request.
