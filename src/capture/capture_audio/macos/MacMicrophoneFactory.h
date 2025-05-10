#pragma once

#include "IMicrophoneFactory.h"

namespace capture::capture_audio::macos
{

class MacMicrophoneFactory final : public IMicrophoneFactory
{
public:
    auto createMicrophone(
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IMicrophone> override;
};

}; // namespace capture::capture_audio::macos
