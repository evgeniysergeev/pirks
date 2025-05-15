#pragma once

#include <vector>

#include "CaptureResult.h"

namespace capture::capture_audio
{

class IAudioInput
{
public:
    virtual ~IAudioInput() = default;

public:
    virtual auto sample(std::vector<float> &sample_in) -> CaptureResult = 0;
};

}; // namespace capture::capture_audio
