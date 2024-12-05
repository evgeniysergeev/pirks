#include <gtest/gtest.h>

#include "CircularBuffer.h"

using namespace std::literals;

template<class T>
class CircularBufferEx : public CircularBuffer<T>
{
public:
    explicit CircularBufferEx(size_t maxElements = 256) : CircularBuffer<T>(maxElements) {};
};

TEST(CircularBuffer, Initialization)
{
    CircularBuffer<int> buff;
    EXPECT_TRUE(buff.isEmpty());
    EXPECT_FALSE(buff.isFull());
    EXPECT_TRUE(buff.isActive());
    EXPECT_EQ(buff.peek(), std::nullopt);
    EXPECT_EQ(buff.peek(100ms), std::nullopt);
}

TEST(CircularBuffer, BufferStopped)
{
    CircularBuffer<int> buff(32);
    EXPECT_EQ(buff.capacity(), 32);

    buff.stop();
    EXPECT_TRUE(buff.isEmpty());
    EXPECT_FALSE(buff.isFull());
    EXPECT_FALSE(buff.isActive());
    EXPECT_EQ(buff.capacity(), 32);
    EXPECT_EQ(buff.peek(), std::nullopt);
    EXPECT_EQ(buff.peek(100ms), std::nullopt);
}

TEST(CircularBuffer, PrintEmptyBuff)
{
    CircularBufferEx<int> buff(2);
}
