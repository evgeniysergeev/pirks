/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <memory>
#include <string>

#include "AudioDeviceEnumerator.h"
#include "AudioNotificationImpl.h"
#include "EndpointNotificationRegistration.h"
#include "IAudioInput.h"
#include "Interface.h"
#include "MmcssTaskHandle.h"
#include "WasapiAudioClient.h"
#include "WinHandle.h"

namespace audio::capture_audio::platform_windows
{

class WasapiAudioInput final: public IAudioInput
{
public:
    WasapiAudioInput(uint8_t channels, uint32_t sample_rate, uint32_t frame_size);

    WasapiAudioInput(
            uint8_t            channels,
            uint32_t           sample_rate,
            uint32_t           frame_size,
            const std::string &audio_source);

    ~WasapiAudioInput() override;

public:
    auto sample(std::vector<float> &sample_out) -> CaptureResult override;

private:
    void initialize(
            uint8_t            channels,
            uint32_t           sample_rate,
            uint32_t           frame_size,
            const std::string &audio_source);

    auto fillBuffer() -> CaptureResult;

private:
    pirks::platform_windows::NullWinHandle audioEvent_;

    AudioDeviceEnumerator                         deviceEnumerator_ {};
    pirks::platform_windows::Interface<IMMDevice> device_;

    // TODO: remove unique_ptr and create needed constructors for this
    std::unique_ptr<WasapiAudioClient>                      audioClient_;
    pirks::platform_windows::Interface<IAudioCaptureClient> audioCapture_;

    DWORD defaultLatency_ {}; // in milliseconds;

    std::vector<float> buffer_;
    float             *bufferPos_ {};
    uint8_t            channels_ {};

    AudioNotificationImpl            audioNotification_;
    EndpointNotificationRegistration endpointNotificationRegistration_;
    // TODO: std::optional<std::function<void()>> default_endpt_changed_cb;

    MmcssTaskHandle mmcssTaskHandle_;
};

}; // namespace audio::capture_audio::platform_windows
