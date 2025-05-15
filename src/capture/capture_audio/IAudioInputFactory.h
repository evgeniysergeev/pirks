#pragma once

#include <memory>

#include "IAudioInput.h"

namespace capture::capture_audio
{

class IAudioInputFactory
{
public:
    virtual ~IAudioInputFactory() = default;

public:
    virtual auto
    create(int                 channels,
           std::uint32_t       sample_rate,
           std::uint32_t       frame_size,
           const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> = 0;
};

}; // namespace capture::capture_audio
