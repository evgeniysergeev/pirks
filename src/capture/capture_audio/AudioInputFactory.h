#pragma once

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
