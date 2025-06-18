#pragma once

/*
 * Includes in this file are platform dependent. Cppcheck cannot find windows
 * includes on MacOS, etc. So just disable all cppcheck checks for missing includes.
 */
// cppcheck-suppress-begin missingInclude

#ifdef LINUX
#include "LinuxAudioInputFactory.h"
namespace audio::capture_audio
{
using AudioInputFactory = ::audio::capture_audio::windows::LinuxAudioInputFactory;
}; // namespace audio::capture_audio
#endif // ifdef LINUX

#ifdef WINDOWS
#include "WinAudioInputFactory.h"
namespace audio::capture_audio
{
using AudioInputFactory = ::audio::capture_audio::windows::WinAudioInputFactory;
}; // namespace audio::capture_audio
#endif // ifdef WINDOWS

#ifdef MACOS
#include "MacAudioInputFactory.h"
namespace audio::capture_audio
{
using AudioInputFactory = ::audio::capture_audio::macos::MacAudioInputFactory;
}; // namespace audio::capture_audio
#endif // ifdef MACOS

// cppcheck-suppress-end missingInclude
