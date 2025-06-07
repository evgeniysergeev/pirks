#pragma once

/*
 * Includes in this file are platform dependent. Cppcheck cannot find windows
 * includes on MacOS, etc. So just disable all cppcheck checks for missing includes.
 */
// cppcheck-suppress-begin missingInclude

#ifdef WINDOWS
#include "WinAudioInputFactory.h"
namespace capture::capture_audio
{
using AudioInputFactory = ::capture::capture_audio::windows::WinAudioInputFactory;
}; // namespace capture::capture_audio
#endif

#ifdef MACOS
#include "MacAudioInputFactory.h"
namespace capture::capture_audio
{
using AudioInputFactory = ::capture::capture_audio::macos::MacAudioInputFactory;
}; // namespace capture::capture_audio
#endif

// cppcheck-suppress-end missingInclude
