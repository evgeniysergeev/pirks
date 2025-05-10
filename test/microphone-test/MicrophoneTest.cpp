#include <gtest/gtest.h>

#include "MicrophoneFactory.h"

TEST(Microphone, MicophoneFactory)
{
    using namespace capture::capture_audio;

    MicrophoneFactory micFactory;
    auto              mic = micFactory.createMicrophone(1, 48000, 96000, nullptr);

    ASSERT_TRUE(mic != nullptr);
}
