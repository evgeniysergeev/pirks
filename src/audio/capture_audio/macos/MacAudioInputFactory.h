#pragma once

#include "IAudioInputFactory.h"

namespace audio::capture_audio::platform_macos
{

/**
 * @brief Class for creating audio input sources in macOS
 *
 * Die to system limitations on a macOS, it is not possible to get system sounds
 * from a desktop. But, you can install third-party software for this.
 * For example "BlackHole": https://github.com/ExistentialAudio/BlackHole
 * or "SoundFlower": https://github.com/mattingalls/Soundflower/releases/
 *
 * For "BlackHole", you can use already built cask from homebrew:
 * ```
 *      brew install --cask blackhole-2ch
 * ```
 * And use "BlackHole 2ch" audio input source in create() function, for example.
 *
 * The same for "SoundFlower":
 * ```
 *       brew install --cask soundflower
 * ```
 */
class MacAudioInputFactory final : public IAudioInputFactory
{
public:
    auto getAudioSources() -> std::vector<std::string> override;

    auto create(
            const std::string  &audio_source,
            int                 channels,
            std::uint32_t       sample_rate,
            std::uint32_t       frame_size,
            const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput> override;
};

}; // namespace audio::capture_audio::platform_macos
