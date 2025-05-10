#include "WinMicrophoneFactory.h"

namespace capture::capture_audio::windows
{

auto WinMicrophoneFactory::createMicrophone(
        const std::uint8_t *mapping,
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size) -> std::unique_ptr<IMicrophone>
{
    return nullptr;
}

}; // namespace capture::capture_audio::windows
