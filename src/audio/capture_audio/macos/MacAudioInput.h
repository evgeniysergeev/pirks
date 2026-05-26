#pragma once

#include <AVFoundation/AVFoundation.h>

#include "CaptureDevice.h"
#include "IAudioInput.h"

class MacAudioInput final: public IAudioInput
{
public:
    MacAudioInput(
            AVCaptureDevice *capture_device,
            uint8_t          channels,
            uint32_t         sample_rate,
            uint32_t         frame_size);

    ~MacAudioInput() override;

public:
    auto sample(std::vector<float> &sample_out) -> CaptureResult override;

private:
    CaptureDevice *captureDevice_ {};
};
