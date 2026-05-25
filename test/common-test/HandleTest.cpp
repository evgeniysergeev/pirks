#include <gtest/gtest.h>

#include <utility>

#include "WinHandle.h"

namespace
{

struct ZeroInvalidValue
{
    constexpr int operator()() const noexcept
    {
        return 0;
    }
};

struct CountingCloser
{
    void operator()(int handle) const noexcept
    {
        ++closeCount;
        lastClosed = handle;
    }

    static void reset()
    {
        closeCount = 0;
        lastClosed = 0;
    }

    static inline int closeCount {};
    static inline int lastClosed {};
};

using TestHandle = pirks::platform_windows::UniqueHandle<int, ZeroInvalidValue, CountingCloser>;

class UniqueHandleTest: public testing::Test
{
protected:
    void SetUp() override
    {
        CountingCloser::reset();
    }
};

} // namespace

TEST_F(UniqueHandleTest, DefaultHandleIsInvalid)
{
    TestHandle handle;

    EXPECT_FALSE(handle);
    EXPECT_EQ(handle.get(), 0);
    EXPECT_EQ(CountingCloser::closeCount, 0);
}

TEST_F(UniqueHandleTest, ResetClosesPreviousHandle)
{
    {
        TestHandle handle { 10 };

        handle.reset(20);

        EXPECT_TRUE(handle);
        EXPECT_EQ(handle.get(), 20);
        EXPECT_EQ(CountingCloser::closeCount, 1);
        EXPECT_EQ(CountingCloser::lastClosed, 10);
    }

    EXPECT_EQ(CountingCloser::closeCount, 2);
    EXPECT_EQ(CountingCloser::lastClosed, 20);
}

TEST_F(UniqueHandleTest, ReleaseDetachesHandle)
{
    {
        TestHandle handle { 10 };

        EXPECT_EQ(handle.release(), 10);
        EXPECT_FALSE(handle);
    }

    EXPECT_EQ(CountingCloser::closeCount, 0);
}

TEST_F(UniqueHandleTest, MoveTransfersOwnership)
{
    {
        TestHandle original { 10 };
        TestHandle moved { std::move(original) };

        EXPECT_FALSE(original);
        EXPECT_TRUE(moved);
        EXPECT_EQ(moved.get(), 10);
    }

    EXPECT_EQ(CountingCloser::closeCount, 1);
    EXPECT_EQ(CountingCloser::lastClosed, 10);
}

TEST(WinHandle, NullWinHandleUsesNullInvalidValue)
{
    pirks::platform_windows::NullWinHandle handle;

    EXPECT_FALSE(handle);
    EXPECT_EQ(handle.get(), nullptr);
}

TEST(WinHandle, NullWinHandleOwnsCreateEventHandle)
{
    pirks::platform_windows::NullWinHandle event = CreateEventA(nullptr, FALSE, FALSE, nullptr);

    ASSERT_TRUE(event);
    EXPECT_NE(event.get(), nullptr);
}
