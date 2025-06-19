#pragma once

#include <vector>

#include "CaptureResult.h"

namespace audio::capture_audio
{

/**
 * @brief Interface for capturing audio samples
 *
 */
class IAudioInput
{
public:
    virtual ~IAudioInput() = default;

public:
    /**
     * @brief Capture audio sample
     *
     * @param sample_in         Preallocated buffer for sample data
     * @return CaptureResult    OK if all good
     */
    virtual auto sample(std::vector<float> &sample_in) -> CaptureResult = 0;
};

}; // namespace audio::capture_audio
