#pragma once

#ifdef WINDOWS
#include "WinMicrophoneFactory.h"
namespace capture::capture_audio
{
using MicrophoneFactory = ::capture::capture_audio::windows::WinMicrophoneFactory;
}; // namespace capture::capture_audio
#endif

#ifdef MACOS
#include "MacMicrophoneFactory.h"
namespace capture::capture_audio
{
using MicrophoneFactory = ::capture::capture_audio::macos::MacMicrophoneFactory;
}; // namespace capture::capture_audio
#endif
