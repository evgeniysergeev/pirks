#pragma once

#include <expected>
#include <vector>

#include "CaptureResult.h"

namespace capture::capture_audio
{

class IMicrophone
{
public:
    virtual ~IMicrophone() = default;

public:
    virtual auto sample() -> std::expected<std::vector<std::uint16_t>, CaptureResult> = 0;
};

}; // namespace capture::capture_audio
