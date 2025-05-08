#pragma once

#include <expected>

namespace capture_audio
{

class IMicrophone
{
public:
    virtual ~IMicrophone() = default;

public:
    virtual auto sample() -> std::expected<std::vector<std::uint16_t>, int> = 0;
};

}; // namespace capture_audio
