/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include "AudioClientPtr.h"
#include "AudioNotificationImpl.h"
#include "DeviceEnumeratorPtr.h"
#include "IAudioInput.h"
#include "Interface.h"
#include "WinHandle.h"

namespace audio::capture_audio::platform_windows
{

class WasapiAudioInput final: public IAudioInput
{
public:
    WasapiAudioInput(uint8_t channels, uint32_t sample_rate, uint32_t frame_size);

    ~WasapiAudioInput() override;

public:
    auto sample(std::vector<float> &sample_out) -> CaptureResult override;

private:
    auto fillBuffer() -> CaptureResult;

private:
    pirks::platform_windows::WinHandle audioEvent_;

    DeviceEnumeratorPtr                           deviceEnumerator_ {};
    pirks::platform_windows::Interface<IMMDevice> device_;

    // TODO: remove unique_ptr and create needed constructors for this
    std::unique_ptr<AudioClientPtr>                         audioClient_;
    pirks::platform_windows::Interface<IAudioCaptureClient> audioCapture_;

    DWORD defaultLatency_; // in milliseconds;

    std::vector<float> buffer_;
    float             *bufferPos_;
    uint8_t            channels_;

    AudioNotificationImpl audioNotification_;
    // TODO: std::optional<std::function<void()>> default_endpt_changed_cb;

    // TODO: create class for this
    pirks::platform_windows::WinHandle mmcss_task_handle_;
};

}; // namespace audio::capture_audio::platform_windows
