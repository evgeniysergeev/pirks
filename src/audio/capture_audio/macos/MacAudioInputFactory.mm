#include "MacAudioInputFactory.h"
#include "MacAudioInput.h"

#include <string>

namespace audio::capture_audio::platform_macos
{

auto MacAudioInputFactory::getAudioSources() -> std::vector<std::string>
{
    std::vector<std::string> result;

    for (NSString *deviceName in [CaptureDevice captureDeviceNames]) {
        result.push_back([deviceName UTF8String]);
    }

    return result;
}

auto MacAudioInputFactory::create(
        const std::string  &audio_source,
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size,
        const std::uint8_t * /* mapping */) -> std::unique_ptr<IAudioInput>
{
    AVCaptureDevice *captureDevice =
        [CaptureDevice findCaptureDevice:[NSString stringWithUTF8String:audio_source.c_str()]];

    if (captureDevice == nullptr) {
        return nullptr;
    }
    
    try {
        return std::make_unique<MacAudioInput>(captureDevice, channels, sample_rate, frame_size);
    } catch (const std::exception &e) {
        return nullptr;
    }
}

}; // namespace audio::capture_audio::platform_macos
