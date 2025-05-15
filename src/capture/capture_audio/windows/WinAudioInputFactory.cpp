#include "WinAudioInputFactory.h"

namespace capture::capture_audio::windows
{

auto WinAudioInputFactory::create(
        const std::uint8_t *mapping,
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size) -> std::unique_ptr<IAudioInput>
{
    return nullptr;
}

}; // namespace capture::capture_audio::windows
