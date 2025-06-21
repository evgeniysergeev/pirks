#include <gtest/gtest.h>

#include "AudioInputFactory.h"

constexpr auto kSamplesCount = 8;

TEST(AudioInput, CaptureDevice)
{
    using namespace audio::capture_audio;

    AudioInputFactory audio_input_factory;

    const auto names = audio_input_factory.getAudioSources();
    ASSERT_GE(names.size(), 1u);

    auto device = audio_input_factory.create(names.at(0), 1, 48000, 96000, nullptr);
    ASSERT_TRUE(device != nullptr);
}

TEST(AudioInput, GetSamples)
{
    using namespace audio;
    using namespace audio::capture_audio;

    AudioInputFactory audio_input_factory;
    const auto        names = audio_input_factory.getAudioSources();

    std::vector<float> sample_in(1024);

    for (const auto &name: names) {
        auto device = audio_input_factory.create(name, 2, 48000, 96000, nullptr);
        ASSERT_TRUE(device != nullptr);

        for (auto i = 0; i < kSamplesCount; i++) {
            auto result = device->sample(sample_in);
            ASSERT_TRUE(result == CaptureResult::OK);
        }
    }
}
