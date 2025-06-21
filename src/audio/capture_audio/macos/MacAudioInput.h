#pragma once

#include <AVFoundation/AVFoundation.h>

#include "CaptureDevice.h"
#include "IAudioInput.h"

namespace audio::capture_audio::platform_macos
{

class MacAudioInput final: public IAudioInput
{
public:
    MacAudioInput(
            AVCaptureDevice *capture_device,
            int              channels,
            uint32_t         sample_rate,
            uint32_t         frame_size);

    ~MacAudioInput() override;

public:
    auto sample(std::vector<float> &sample_in) -> CaptureResult override;

private:
    CaptureDevice *captureDevice_ {};
};

}; // namespace audio::capture_audio::platform_macos
