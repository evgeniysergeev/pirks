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
    virtual std::unique_ptr<IMicrophone> createMicrophone(
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) = 0;
};

}; // namespace capture::capture_audio
