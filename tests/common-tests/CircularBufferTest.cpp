#include <gtest/gtest.h>

#include "CircularBuffer.h"

using namespace std::literals;

TEST(CircularBufferEmptyTest, BasicAssertions)
{
    CircularBuffer<int> buff;
    EXPECT_TRUE(buff.isEmpty());
    EXPECT_FALSE(buff.isFull());
    EXPECT_TRUE(buff.isActive());
    EXPECT_EQ(buff.peek(), std::nullopt);
    EXPECT_EQ(buff.peek(100ms), std::nullopt);
}
