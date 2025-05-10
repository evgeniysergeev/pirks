#pragma once

#include <memory>

#include "IMicrophone.h"

namespace capture::capture_audio
{

class IMicrophoneFactory
{
public:
    virtual ~IMicrophoneFactory() = default;

public:
    virtual auto createMicrophone(
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IMicrophone> = 0;
};

}; // namespace capture::capture_audio
