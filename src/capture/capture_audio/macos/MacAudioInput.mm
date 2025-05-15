#include "MacAudioInput.h"

#include <stdexcept>

namespace capture::capture_audio::macos
{

MacAudioInput::MacAudioInput(AVCaptureDevice *capture_device, int channels, uint32_t sample_rate, uint32_t frame_size)
{
    captureDevice_ = [[CaptureDevice alloc] init];

    if ([captureDevice_ setupCaptureDevice:capture_device
                       sampleRate:sample_rate
                        frameSize:frame_size
                         channels:channels])
    {
        throw std::runtime_error("Failed to setup microphone");
    }
}

MacAudioInput::~MacAudioInput()
{
    [captureDevice_ release];
}

auto MacAudioInput::sample(std::vector<float> &sample_in) -> CaptureResult
{
    const auto sample_size = sample_in.size();

    uint32_t length = 0;
    void *byteSampleBuffer = TPCircularBufferTail(&captureDevice_->audioSampleBuffer, &length);

    while (length < sample_size * sizeof(float)) {
        [captureDevice_.samplesArrivedSignal wait];
        byteSampleBuffer = TPCircularBufferTail(&captureDevice_->audioSampleBuffer, &length);
    }

    const float *sampleBuffer = reinterpret_cast<float *>(byteSampleBuffer);
    std::vector<float> vectorBuffer(sampleBuffer, sampleBuffer + sample_size);

    std::copy_n(std::begin(vectorBuffer), sample_size, std::begin(sample_in));

    TPCircularBufferConsume(&captureDevice_->audioSampleBuffer, sample_size * sizeof(float));

    return CaptureResult::OK;
}

}; // namespace capture::capture_audio::macos
