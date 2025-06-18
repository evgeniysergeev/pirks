#include <gtest/gtest.h>

#include "AudioInputFactory.h"

TEST(AudioInput, MicophoneDevice)
{
    using namespace audio::capture_audio;

    AudioInputFactory audio_input_factory;

    const auto names = audio_input_factory.getMicrophoneNames();
    ASSERT_GE(names.size(), 1u);

    auto mic = audio_input_factory.create(names.at(0), 1, 48000, 96000, nullptr);
    ASSERT_TRUE(mic != nullptr);
}

