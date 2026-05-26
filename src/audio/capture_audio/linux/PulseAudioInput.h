#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "IAudioInput.h"

struct pa_simple;

namespace audio::capture_audio::platform_linux
{

struct PulseAudioSimpleDeleter final
{
    void operator()(pa_simple *stream) const noexcept;
};

class PulseAudioInput final: public IAudioInput
{
public:
    PulseAudioInput(
            const std::string  &source_name,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping);

    ~PulseAudioInput() override;

public:
    auto sample(std::vector<float> &sample_out) -> CaptureResult override;

private:
    std::unique_ptr<pa_simple, PulseAudioSimpleDeleter> stream_;
};

}; // namespace audio::capture_audio::platform_linux
