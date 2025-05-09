#pragma once

#include <AVFoundation/AVFoundation.h>

#include "AVAudio.h"
#include "IMicrophone.h"

namespace capture::capture_audio::macos
{

class MacMicrophone final : public IMicrophone
{
public:
    MacMicrophone(AVCaptureDevice *capture_device, int channels, uint32_t sample_rate, uint32_t frame_size);

    ~MacMicrophone() override;

public:
    CaptureResult sample(std::vector<float> &sample_in) override;

private:
    AVAudio *avAudio_ {};
};

}; // namespace capture::capture_audio::macos
