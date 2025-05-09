#include "WinMicrophoneFactory.h"

namespace capture::capture_audio::windows
{

std::unique_ptr<IMicrophone> WinMicrophoneFactory::createMicrophone(
        const std::uint8_t *mapping,
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size)
{
    return nullptr;
}

}; // namespace capture::capture_audio::windows
