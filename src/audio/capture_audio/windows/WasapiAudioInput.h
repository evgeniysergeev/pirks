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

    DeviceEnumeratorPtr                                     deviceEnumerator_;
    pirks::platform_windows::Interface<IMMDevice>           device_;
    AudioClientPtr                                          audioClient_;
    pirks::platform_windows::Interface<IAudioCaptureClient> audioCapture_;

    REFERENCE_TIME defaultLatency; // in milliseconds;

    std::vector<float> buffer_;
    float             *bufferPos_;
    int                channels_;

    AudioNotification audioNotification_;
    /*
    std::optional<std::function<void()>> default_endpt_changed_cb;


    HANDLE mmcss_task_handle = NULL;
    */
};

}; // namespace audio::capture_audio::platform_windows
