/*
 * Centralized definitions for Windows audio-related GUIDs/IIDs.
 */

#pragma once

#include <audioclient.h>
#include <mmdeviceapi.h>

namespace audio::capture_audio::platform_windows
{

inline const CLSID CLSID_MMDeviceEnumerator    = __uuidof(MMDeviceEnumerator);
inline const IID   IID_IMMDeviceEnumerator     = __uuidof(IMMDeviceEnumerator);

inline const IID IID_IAudioClient              = __uuidof(IAudioClient);
inline const IID IID_IAudioCaptureClient       = __uuidof(IAudioCaptureClient);
inline const IID IID_IMMNotificationClient     = __uuidof(IMMNotificationClient);

}; // namespace audio::capture_audio::platform_windows

