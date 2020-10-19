@echo off
setlocal enableextensions 

pushd %~dp0

set arch=%1
if "%arch%" == "" (set arch=x64)
if "%arch%" == "x64" (set vs_arch=x64)
if "%arch%" == "x86" (set vs_arch=Win32)
set source_dir=%cd%
set build_dir=_build-%arch%

echo Target architecture: %arch% (%vs_arch%)
echo Source directory:    %source_dir%
echo Build directory      %build_dir%

git submodule update --init

pushd vcpkg
call bootstrap-vcpkg.bat
vcpkg install @%source_dir%\cmake\vcpkg_%arch%-windows.txt 
popd

mkdir %build_dir%

for /f "usebackq tokens=*" %%a in (`where /R . cmake.exe`) do (set cmake=%%a; goto endfor)
:endfor

if "%cmake%" == "" (
	echo CMake not found. You should install it.
	goto end)
%cmake% -S %source_dir% -B %build_dir% -A %vs_arch% -DCMAKE_TOOLCHAIN_FILE=%source_dir%\vcpkg\scripts\buildsystems\vcpkg.cmake

:end
popd