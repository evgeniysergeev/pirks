#pragma once

#include "IAudioInputFactory.h"

namespace audio::capture_audio::macos
{

class MacAudioInputFactory final : public IAudioInputFactory
{
public:
    auto create(
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> override;
};

}; // namespace audio::capture_audio::macos
