#include "MacAudioInputFactory.h"
#include "MacAudioInput.h"

namespace audio::capture_audio::macos
{

auto MacAudioInputFactory::create(
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size,
        const std::uint8_t * /* mapping */) -> std::unique_ptr<IAudioInput>
{
    const char *audio_sink = "";

    // TODO: 
    /*
      if (!config::audio.sink.empty()) {
        audio_sink = config::audio.sink.c_str();
      }
    */

    AVCaptureDevice *captureDevice = [CaptureDevice findCaptureDevice:[NSString stringWithUTF8String:audio_sink]
                                                          microphone:TRUE];
    if (captureDevice == nullptr) {
        NSString *audioSinkName = nullptr;
        for (NSString *name in [CaptureDevice captureDeviceNames:TRUE]) {
            audioSinkName = name;
            break;
        }

        captureDevice = [CaptureDevice findCaptureDevice:audioSinkName
                                             microphone:TRUE];

    // TODO: 
    /*
        BOOST_LOG(error) << "opening AudioInput '"sv << audio_sink << "' failed. Please set a valid input source in the Sunshine config."sv;
        BOOST_LOG(error) << "Available inputs:"sv;
        */

    // TODO: 
    /*
        for (NSString *name in [CaptureDevice AudioInputNames]) {
          BOOST_LOG(error) << "\t"sv << [name UTF8String];
        }
        */

        if (captureDevice == nullptr) {
            return nullptr;
        }
    }
    
    try {
        return std::make_unique<MacAudioInput>(captureDevice, channels, sample_rate, frame_size);
    } catch (const std::exception &e) {
        return nullptr;
    }
}

}; // namespace audio::capture_audio::macos
