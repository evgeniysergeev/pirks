#include <gtest/gtest.h>

#include "AudioInputFactory.h"
#include "ComInitializer.h"

constexpr auto kSamplesCount = 8;

TEST(AudioInput, CaptureDevice)
{
#ifdef WINDOWS
#include "ComInitializer.h"

    // Ensure COM is initialized for Windows microphone tests
    static ::pirks::platform_windows::ComInitializer g_com_initializer;
#endif

    using namespace audio::capture_audio;

    AudioInputFactory audio_input_factory;

    const auto names = audio_input_factory.getAudioSources();
    ASSERT_GE(names.size(), 1u);

    auto device = audio_input_factory.create(names.at(0), 2, 48000, 288000, nullptr);
    if (!device) {
        GTEST_SKIP() << "No accessible audio capture device found";
    }
}

TEST(AudioInput, GetSamples)
{
#ifdef WINDOWS
#include "ComInitializer.h"

    // Ensure COM is initialized for Windows microphone tests
    static ::pirks::platform_windows::ComInitializer g_com_initializer;
#endif

    using namespace audio;
    using namespace audio::capture_audio;

    AudioInputFactory audio_input_factory;
    const auto        names = audio_input_factory.getAudioSources();

    std::vector<float> sample_in(1024);
    bool               sampled_device = false;

    for (const auto &name: names) {
        auto device = audio_input_factory.create(name, 2, 48000, 96000, nullptr);
        if (!device) {
            continue;
        }

        sampled_device = true;

        for (auto i = 0; i < kSamplesCount; i++) {
            auto result = device->sample(sample_in);
            ASSERT_TRUE(result == CaptureResult::OK);
        }
    }

    if (!sampled_device) {
        GTEST_SKIP() << "No accessible audio capture device found";
    }
}
