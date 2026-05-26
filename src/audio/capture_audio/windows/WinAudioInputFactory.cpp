#include "WinAudioInputFactory.h"

#include "AudioDeviceEnumerator.h"
#include "WasapiAudioInput.h"

namespace audio::capture_audio::platform_windows
{

auto WinAudioInputFactory::getAudioSources() -> std::vector<std::string>
{
    AudioDeviceEnumerator enumerator;
    return enumerator.getDeviceNames();
}

auto WinAudioInputFactory::getDefaultAudioSourceName() -> std::optional<std::string>
{
    AudioDeviceEnumerator enumerator;
    return enumerator.getDefaultDeviceName();
}

auto WinAudioInputFactory::create(
        const std::string &audio_source,
        int                channels,
        std::uint32_t      sample_rate,
        std::uint32_t      frame_size,
        const std::uint8_t * /* mapping */) -> std::unique_ptr<IAudioInput>
{
    try {
        if (audio_source.empty()) {
            return nullptr;
        }

        return std::make_unique<WasapiAudioInput>(
                static_cast<std::uint8_t>(channels),
                sample_rate,
                frame_size,
                audio_source);
    } catch (const std::exception &) {
        return nullptr;
    }
}

}; // namespace audio::capture_audio::platform_windows
