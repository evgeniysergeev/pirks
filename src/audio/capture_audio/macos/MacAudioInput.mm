#include "MacAudioInput.h"

#include <stdexcept>

namespace audio::capture_audio::platform_macos
{

MacAudioInput::MacAudioInput(AVCaptureDevice *capture_device, uint8_t channels, uint32_t sample_rate, uint32_t frame_size)
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

auto MacAudioInput::sample(std::vector<float> &sample_out) -> CaptureResult
{
    const auto sample_size64 = sample_out.size();
    assert(sample_size64 < UINT32_MAX && "audio sample buffer overflow");
    const uint32_t sample_size = static_cast<uint32_t>(sample_size64);

    uint32_t length = 0;
    void *byteSampleBuffer = TPCircularBufferTail(&captureDevice_->audioSampleBuffer, &length);

    while (length < sample_size * sizeof(float)) {
        [captureDevice_.samplesArrivedSignal wait];
        byteSampleBuffer = TPCircularBufferTail(&captureDevice_->audioSampleBuffer, &length);
    }

    const float *sampleBuffer = reinterpret_cast<float *>(byteSampleBuffer);
    std::vector<float> vectorBuffer(sampleBuffer, sampleBuffer + sample_size);

    std::copy_n(std::begin(vectorBuffer), sample_size, std::begin(sample_out));

    TPCircularBufferConsume(&captureDevice_->audioSampleBuffer, sample_size * sizeof(float));

    return CaptureResult::OK;
}

}; // namespace audio::capture_audio::platform_macos
