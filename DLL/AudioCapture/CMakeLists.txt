cmake_minimum_required(VERSION 2.8)

# include("dll.cmake")

add_library(AudioCapture SHARED
	AnalyseAudio.cpp
	# AudioCapture.cpp
	AudioFormats.cpp
	Globals.cpp
	hardware.cpp
	RecordAudio.cpp
	Settings.cpp
	stdafx.cpp
	StringFunctions.cpp
	WinlircAudioIn.cpp

    AudioCapture.rc

    AnalyseAudio.h
    AudioFormats.h
    Globals.h
    RecordAudio.h
    resource.h
    Settings.h
    stdafx.h
    StringFunctions.h
    targetver.h
	)

target_link_libraries (AudioCapture
	winlirc
	)

configure_plugin(AudioCapture)
