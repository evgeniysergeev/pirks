#include "MacAudioInputFactory.h"

#include "MacAudioInput.h"

#include <algorithm>
#include <exception>
#include <optional>
#include <string>

namespace audio::capture_audio::platform_macos
{

auto MacAudioInputFactory::getAudioSources() -> std::vector<std::string>
{
    std::vector<std::string> result;

    for (NSString *deviceName in [CaptureDevice captureDeviceNames]) {
        const std::string current_name = [deviceName UTF8String];
        if (std::find(result.begin(), result.end(), current_name) == result.end()) {
            result.push_back(current_name);
        }
    }

    return result;
}

auto MacAudioInputFactory::getDefaultAudioSourceName() -> std::optional<std::string>
{
    AVCaptureDevice *captureDevice = [CaptureDevice defaultCaptureDevice];
    if (captureDevice == nullptr) {
        return std::nullopt;
    }

    return [[captureDevice localizedName] UTF8String];
}

auto MacAudioInputFactory::create(
        const std::string  &audio_source,
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size,
        const std::uint8_t * /* mapping */) -> std::unique_ptr<IAudioInput>
{
    if (audio_source.empty()) {
        return nullptr;
    }

    AVCaptureDevice *captureDevice =
            [CaptureDevice findCaptureDevice:[NSString stringWithUTF8String:audio_source.c_str()]];

    if (captureDevice == nullptr) {
        return nullptr;
    }

    try {
        return std::make_unique<MacAudioInput>(captureDevice, channels, sample_rate, frame_size);
    } catch (const std::exception &) {
        return nullptr;
    }
}

}; // namespace audio::capture_audio::platform_macos
