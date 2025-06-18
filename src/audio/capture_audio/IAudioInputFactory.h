#pragma once

#include <memory>

#include "IAudioInput.h"

namespace audio::capture_audio
{

class IAudioInputFactory
{
public:
    virtual ~IAudioInputFactory() = default;

public:
    virtual auto create(
            const std::string  &sink_name,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> = 0;
};

}; // namespace audio::capture_audio
