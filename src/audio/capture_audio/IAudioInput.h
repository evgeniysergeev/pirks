#pragma once

#include <vector>

#include "CaptureResult.h"

namespace audio::capture_audio
{

class IAudioInput
{
public:
    virtual ~IAudioInput() = default;

public:
    virtual auto sample(std::vector<float> &sample_in) -> CaptureResult = 0;
};

}; // namespace audio::capture_audio
