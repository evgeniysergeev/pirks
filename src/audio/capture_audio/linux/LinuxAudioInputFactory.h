#pragma once

#include "IAudioInputFactory.h"

namespace audio::capture_audio::platform_linux
{

class LinuxAudioInputFactory final: public IAudioInputFactory
{
public:
    auto getAudioSources() -> std::vector<std::string> override;

    auto create(
            const std::string  &audio_source,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> override;
};

}; // namespace audio::capture_audio::platform_linux
