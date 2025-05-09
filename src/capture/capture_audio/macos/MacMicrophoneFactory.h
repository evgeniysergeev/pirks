#pragma once

#include "IMicrophoneFactory.h"

namespace capture::capture_audio::macos
{

class MacMicrophoneFactory final : public IMicrophoneFactory
{
public:
    std::unique_ptr<IMicrophone> createMicrophone(
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) override;
};

}; // namespace capture::capture_audio::macos
