#pragma once

/*
 * Includes in this file are platform dependent. Cppcheck cannot find windows
 * includes on MacOS, etc. So just disable all cppcheck checks for missing includes.
 */
// cppcheck-suppress-begin missingInclude

#ifdef LINUX
#include "LinuxAudioInputFactory.h"
using AudioInputFactory = LinuxAudioInputFactory;
#endif // ifdef LINUX

#ifdef WINDOWS
#include "WinAudioInputFactory.h"
using AudioInputFactory = WinAudioInputFactory;
#endif // ifdef WINDOWS

#ifdef MACOS
#include "MacAudioInputFactory.h"
using AudioInputFactory = MacAudioInputFactory;
#endif // ifdef MACOS

// cppcheck-suppress-end missingInclude
