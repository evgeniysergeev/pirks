#pragma once

#include <vector>

#include "CaptureResult.h"

namespace capture::capture_audio
{

class IMicrophone
{
public:
    virtual ~IMicrophone() = default;

public:
    virtual CaptureResult sample(std::vector<float> &sample_in) = 0;
};

}; // namespace capture::capture_audio
