#include <gtest/gtest.h>

#include "CircularBuffer.h"
#include "debug/CircularBufferToStr.h"

using namespace std::literals;

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
    CircularBuffer<int> buff(31);
    EXPECT_EQ(buff.bufferCapacity(), 32u);

    buff.stop();
    EXPECT_TRUE(buff.isEmpty());
    EXPECT_FALSE(buff.isFull());
    EXPECT_FALSE(buff.isActive());
    EXPECT_EQ(buff.bufferCapacity(), 32u);
    EXPECT_EQ(buff.peek(), std::nullopt);
    EXPECT_EQ(buff.peek(100ms), std::nullopt);
}

TEST(CircularBuffer, BasicUsage)
{
    CircularBuffer<int> buff { 7 };
    EXPECT_EQ(buff.bufferCapacity(), 8u);

    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    buff.push(1);
    EXPECT_EQ(CircularBufferToStr(buff), "{1}"s);

    buff.push(2);
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2}"s);

    buff.push(3);
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2,3}"s);

    buff.push(4);
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2,3,4}"s);

    EXPECT_EQ(buff.pop(), 1);
    EXPECT_EQ(CircularBufferToStr(buff), "{2,3,4}"s);

    buff.push(5);
    EXPECT_EQ(CircularBufferToStr(buff), "{2,3,4,5}"s);

    EXPECT_EQ(buff.pop(), 2);
    EXPECT_EQ(CircularBufferToStr(buff), "{3,4,5}"s);

    EXPECT_EQ(buff.pop(), 3);
    EXPECT_EQ(CircularBufferToStr(buff), "{4,5}"s);

    {
        const auto value = buff.pop(20ms);
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value, 4);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{5}"s);

    {
        const auto value = buff.pop(20ms);
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value, 5);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    EXPECT_FALSE(buff.pop(20ms).has_value());
    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    EXPECT_FALSE(buff.pop(20ms).has_value());
    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    buff.push(6);
    EXPECT_EQ(CircularBufferToStr(buff), "{6}"s);

    buff.push(7);
    EXPECT_EQ(CircularBufferToStr(buff), "{6,7}"s);

    buff.push(8);
    EXPECT_EQ(CircularBufferToStr(buff), "{6,7,8}"s);

    buff.push(9);
    EXPECT_EQ(CircularBufferToStr(buff), "{6,7,8,9}"s);
}

TEST(CircularBuffer, PeekValue)
{
    CircularBuffer<int> buff { 7 };

    EXPECT_FALSE(buff.peek().has_value());
    EXPECT_FALSE(buff.peek(20ms).has_value());

    buff.push(1);
    buff.push(2);
    buff.push(3);
    buff.push(4);
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2,3,4}"s);

    {
        const auto value = buff.peek();
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value, 1);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2,3,4}"s);

    EXPECT_EQ(buff.pop(), 1);
    EXPECT_EQ(CircularBufferToStr(buff), "{2,3,4}"s);

    {
        const auto value = buff.peek();
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value, 2);
    }
}

TEST(CircularBuffer, Overwrite)
{
    CircularBuffer<int> buff { 3 };

    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    buff.push(1);
    EXPECT_EQ(CircularBufferToStr(buff), "{1}"s);

    buff.push(2);
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2}"s);

    buff.push(3);
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2,3}"s);

    buff.push(4);
    EXPECT_EQ(CircularBufferToStr(buff), "{2,3,4}"s);

    buff.push(5);
    EXPECT_EQ(CircularBufferToStr(buff), "{3,4,5}"s);

    EXPECT_EQ(buff.pop(), 3);
    EXPECT_EQ(CircularBufferToStr(buff), "{4,5}"s);

    buff.push(6);
    EXPECT_EQ(CircularBufferToStr(buff), "{4,5,6}"s);

    EXPECT_EQ(buff.pop(), 4);
    EXPECT_EQ(CircularBufferToStr(buff), "{5,6}"s);

    EXPECT_EQ(buff.pop(), 5);
    EXPECT_EQ(CircularBufferToStr(buff), "{6}"s);

    buff.push(7);
    EXPECT_EQ(CircularBufferToStr(buff), "{6,7}"s);

    EXPECT_EQ(buff.pop(), 6);
    EXPECT_EQ(CircularBufferToStr(buff), "{7}"s);

    EXPECT_EQ(buff.pop(), 7);
    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    EXPECT_FALSE(buff.pop(20ms).has_value());
    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);
}

TEST(CircularBuffer, PushMultipleElements)
{
    CircularBuffer<int> buff { 8 };
    EXPECT_EQ(buff.bufferCapacity(), 9u);

    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    {
        int elements[] = { 1, 2, 3, 4 };
        buff.push(elements);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2,3,4}"s);

    EXPECT_EQ(buff.pop(), 1);
    EXPECT_EQ(CircularBufferToStr(buff), "{2,3,4}"s);

    EXPECT_EQ(buff.pop(), 2);
    EXPECT_EQ(CircularBufferToStr(buff), "{3,4}"s);

    EXPECT_EQ(buff.pop(), 3);
    EXPECT_EQ(CircularBufferToStr(buff), "{4}"s);

    {
        const auto value = buff.pop(20ms);
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value, 4);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    {
        const int elements[] = { 6, 7, 8, 9 };
        buff.push(elements);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{6,7,8,9}"s);

    std::vector<int> v;
    v.push_back(10);
    v.push_back(11);
    v.push_back(12);
    v.push_back(13);
    buff.push(v);
    EXPECT_EQ(CircularBufferToStr(buff), "{6,7,8,9,10,11,12,13}"s);
}

TEST(CircularBuffer, PopMultipleElements)
{
    CircularBuffer<int> buff { 8 };
    EXPECT_EQ(buff.bufferCapacity(), 9u);

    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    {
        int elements[] = { 1, 2, 3, 4 };
        buff.push(elements);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{1,2,3,4}"s);

    {
        std::vector<int> v;
        EXPECT_EQ(buff.pop(v, 2), 2u);
        EXPECT_EQ(v.at(0), 1);
        EXPECT_EQ(v.at(1), 2);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{3,4}"s);

    {
        const int elements[] = { 5, 6, 7, 8, 9 };
        buff.push(elements);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{3,4,5,6,7,8,9}"s);

    {
        std::vector<int> v;
        EXPECT_EQ(buff.pop(v, 4, 20ms), 4u);
        EXPECT_EQ(v.at(0), 3);
        EXPECT_EQ(v.at(1), 4);
        EXPECT_EQ(v.at(2), 5);
        EXPECT_EQ(v.at(3), 6);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{7,8,9}"s);

    {
        std::vector<int> v;
        EXPECT_EQ(buff.pop(v, 16), 3u);
        EXPECT_EQ(v.at(0), 7);
        EXPECT_EQ(v.at(1), 8);
        EXPECT_EQ(v.at(2), 9);
    }
    EXPECT_EQ(CircularBufferToStr(buff), "{}"s);

    {
        std::vector<int> v;
        EXPECT_EQ(buff.pop(v, 16, 20ms), 0u);
    }
}
