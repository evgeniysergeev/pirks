#pragma once

#include "IMicrophoneFactory.h"

namespace capture::capture_audio::windows
{

class WindowsMicrophoneFactory final : public IMicrophoneFactory
{
public:
    std::unique_ptr<IMicrophone> createMicrophone(
            const std::uint8_t *mapping,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size) override;
};

}; // namespace capture::capture_audio::windows
