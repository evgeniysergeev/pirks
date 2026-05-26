#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "IAudioInput.h"

namespace audio::capture_audio
{

class IAudioInputFactory
{
public:
    virtual ~IAudioInputFactory() = default;

public:
    /**
     * @brief Get explicitly selectable audio source names.
     */
    virtual auto getAudioSources() -> std::vector<std::string> = 0;

    /**
     * @brief Get the current default audio source name, if available.
     *
     * The returned name can be passed to create().
     */
    virtual auto getDefaultAudioSourceName() -> std::optional<std::string> = 0;

    /**
     * @brief Create audio input for a named source.
     *
     * @param audio_source Name returned by getAudioSources() or getDefaultAudioSourceName().
     */
    virtual auto create(
            const std::string  &audio_source,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> = 0;
};

}; // namespace audio::capture_audio
