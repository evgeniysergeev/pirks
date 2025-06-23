#pragma once

#include <memory>
#include <string>
#include <vector>

#include "IAudioInput.h"

namespace audio::capture_audio
{

class IAudioInputFactory
{
public:
    virtual ~IAudioInputFactory() = default;

public:
    virtual auto getAudioSources() -> std::vector<std::string> = 0;

    virtual auto create(
            const std::string  &audio_source,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> = 0;
};

}; // namespace audio::capture_audio
