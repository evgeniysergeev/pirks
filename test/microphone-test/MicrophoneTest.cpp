#include <gtest/gtest.h>

#include "AudioInputFactory.h"

TEST(AudioInput, MicophoneFactory)
{
    using namespace capture::capture_audio;

    AudioInputFactory audio_input_factory;
    auto              mic = audio_input_factory.create(1, 48000, 96000, nullptr);

    ASSERT_TRUE(mic != nullptr);
}
