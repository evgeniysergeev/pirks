#include "MacMicrophoneFactory.h"
#include "MacMicrophone.h"

namespace capture::capture_audio::macos
{

auto MacMicrophoneFactory::createMicrophone(
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size,
        const std::uint8_t * /* mapping */) -> std::unique_ptr<IMicrophone>
{
    const char *audio_sink = "";

    // TODO: 
    /*
      if (!config::audio.sink.empty()) {
        audio_sink = config::audio.sink.c_str();
      }
    */

    AVCaptureDevice *captureDevice = [AVAudio findMicrophone:[NSString stringWithUTF8String:audio_sink]];
    if (captureDevice == nullptr) {
        NSString *audioSinkName = nullptr;
        for (NSString *name in [AVAudio microphoneNames]) {
            audioSinkName = name;
            break;
        }

        captureDevice = [AVAudio findMicrophone:audioSinkName];

    // TODO: 
    /*
        BOOST_LOG(error) << "opening microphone '"sv << audio_sink << "' failed. Please set a valid input source in the Sunshine config."sv;
        BOOST_LOG(error) << "Available inputs:"sv;
        */

    // TODO: 
    /*
        for (NSString *name in [AVAudio microphoneNames]) {
          BOOST_LOG(error) << "\t"sv << [name UTF8String];
        }
        */

        if (captureDevice == nullptr) {
            return nullptr;
        }
    }
    
    try {
        return std::make_unique<MacMicrophone>(captureDevice, channels, sample_rate, frame_size);
    } catch (const std::exception &e) {
        return nullptr;
    }
}

}; // namespace capture::capture_audio::macos
