#pragma once

#include "IAudioInputFactory.h"

class WinAudioInputFactory final: public IAudioInputFactory
{
public:
    auto getAudioSources() -> std::vector<std::string> override;

    auto getDefaultAudioSourceName() -> std::optional<std::string> override;

    auto create(
            const std::string  &audio_source,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> override;
};
