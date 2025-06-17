#include "WinAudioInputFactory.h"

namespace audio::capture_audio::windows
{

auto WinAudioInputFactory::create(
        [[maybe_unused]] int                 channels,
        [[maybe_unused]] std::uint32_t       sample_rate,
        [[maybe_unused]] std::uint32_t       frame_size,
        [[maybe_unused]] const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput>
{
    return nullptr;
}

}; // namespace audio::capture_audio::windows
