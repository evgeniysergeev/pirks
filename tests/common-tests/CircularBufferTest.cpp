#include <gtest/gtest.h>

#include "CircularBuffer.h"

TEST(CircularBufferTest, BasicAssertions)
{
    CircularBuffer<int> buff;
    EXPECT_EQ(buff.peek(), std::nullopt);
}
