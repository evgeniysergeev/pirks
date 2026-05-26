/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "AudioDeviceEnumerator.h"
#include "AudioNotificationImpl.h"
#include "ComPtr.h"
#include "EndpointNotificationRegistration.h"
#include "IAudioInput.h"
#include "MmcssTaskHandle.h"
#include "WasapiAudioClient.h"
#include "WinHandle.h"

class WasapiAudioInput final: public IAudioInput
{
public:
    WasapiAudioInput(
            uint8_t            channels,
            uint32_t           sample_rate,
            uint32_t           frame_size,
            const std::string &audio_source);

    ~WasapiAudioInput() override;

public:
    auto sample(std::vector<float> &sample_out) -> CaptureResult override;

private:
    auto fillBuffer() -> CaptureResult;

private:
    NullWinHandle audioEvent_;

    AudioDeviceEnumerator deviceEnumerator_ {};
    MmDevice              device_;

    WasapiAudioClient                                    audioClient_;
    ComPtr<IAudioCaptureClient> audioCapture_;

    DWORD defaultLatency_ {}; // in milliseconds;

    std::vector<float> buffer_;
    float             *bufferPos_ {};
    uint8_t            channels_ {};

    AudioNotificationImpl            audioNotification_;
    EndpointNotificationRegistration endpointNotificationRegistration_;
    // TODO: std::optional<std::function<void()>> default_endpt_changed_cb;

    MmcssTaskHandle mmcssTaskHandle_;
};
