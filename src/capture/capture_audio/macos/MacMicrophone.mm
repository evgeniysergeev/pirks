#include "MacMicrophone.h"

#include <stdexcept>

namespace capture::capture_audio::macos
{

MacMicrophone::MacMicrophone(AVCaptureDevice *capture_device, int channels, uint32_t sample_rate, uint32_t frame_size)
{
    avAudio_ = [[AVAudio alloc] init];

    if ([avAudio_ setupMicrophone:capture_device
                       sampleRate:sample_rate
                        frameSize:frame_size
                         channels:channels])
    {
        throw std::runtime_error("Failed to setup microphone");
    }
}

MacMicrophone::~MacMicrophone()
{
    [avAudio_ release];
}

CaptureResult MacMicrophone::sample(std::vector<float> &sample_in)
{
    const auto sample_size = sample_in.size();

    uint32_t length = 0;
    void *byteSampleBuffer = TPCircularBufferTail(&avAudio_->audioSampleBuffer, &length);

    while (length < sample_size * sizeof(float)) {
        [avAudio_.samplesArrivedSignal wait];
        byteSampleBuffer = TPCircularBufferTail(&avAudio_->audioSampleBuffer, &length);
    }

    const float *sampleBuffer = reinterpret_cast<float *>(byteSampleBuffer);
    std::vector<float> vectorBuffer(sampleBuffer, sampleBuffer + sample_size);

    std::copy_n(std::begin(vectorBuffer), sample_size, std::begin(sample_in));

    TPCircularBufferConsume(&avAudio_->audioSampleBuffer, sample_size * sizeof(float));

    return CaptureResult::OK;
}

}; // namespace capture::capture_audio::macos
