#include <gtest/gtest.h>

#include <utility>

#include "Interface.h"

namespace
{

class RefCountedInterface
{
public:
    auto AddRef() -> ULONG
    {
        ++addRefCount_;
        ++refCount_;
        return refCount_;
    }

    auto Release() -> ULONG
    {
        ++releaseCount_;
        --refCount_;
        return refCount_;
    }

    auto refCount() const -> ULONG
    {
        return refCount_;
    }

    auto addRefCount() const -> ULONG
    {
        return addRefCount_;
    }

    auto releaseCount() const -> ULONG
    {
        return releaseCount_;
    }

private:
    ULONG refCount_ { 1 };
    ULONG addRefCount_ {};
    ULONG releaseCount_ {};
};

} // namespace

TEST(Interface, AttachKeepsOwnedReference)
{
    RefCountedInterface ref_counted;

    {
        auto ptr = pirks::platform_windows::Interface<RefCountedInterface>::attach(&ref_counted);

        EXPECT_EQ(ptr.get(), &ref_counted);
        EXPECT_EQ(ref_counted.addRefCount(), 0UL);
        EXPECT_EQ(ref_counted.refCount(), 1UL);
    }

    EXPECT_EQ(ref_counted.releaseCount(), 1UL);
    EXPECT_EQ(ref_counted.refCount(), 0UL);
}

TEST(Interface, BorrowedConstructorAddsReference)
{
    RefCountedInterface ref_counted;

    {
        pirks::platform_windows::Interface<RefCountedInterface> ptr { &ref_counted };

        EXPECT_EQ(ptr.get(), &ref_counted);
        EXPECT_EQ(ref_counted.addRefCount(), 1UL);
        EXPECT_EQ(ref_counted.refCount(), 2UL);
    }

    EXPECT_EQ(ref_counted.releaseCount(), 1UL);
    EXPECT_EQ(ref_counted.refCount(), 1UL);
}

TEST(Interface, CopyAddsReference)
{
    RefCountedInterface ref_counted;

    {
        auto ptr = pirks::platform_windows::Interface<RefCountedInterface>::attach(&ref_counted);

        {
            pirks::platform_windows::Interface<RefCountedInterface> copy { ptr };

            EXPECT_EQ(copy.get(), &ref_counted);
            EXPECT_EQ(ref_counted.addRefCount(), 1UL);
            EXPECT_EQ(ref_counted.refCount(), 2UL);
        }

        EXPECT_EQ(ref_counted.releaseCount(), 1UL);
        EXPECT_EQ(ref_counted.refCount(), 1UL);
    }

    EXPECT_EQ(ref_counted.releaseCount(), 2UL);
    EXPECT_EQ(ref_counted.refCount(), 0UL);
}

TEST(Interface, MoveTransfersReference)
{
    RefCountedInterface ref_counted;

    {
        auto ptr = pirks::platform_windows::Interface<RefCountedInterface>::attach(&ref_counted);
        pirks::platform_windows::Interface<RefCountedInterface> moved { std::move(ptr) };

        EXPECT_FALSE(ptr);
        EXPECT_EQ(moved.get(), &ref_counted);
        EXPECT_EQ(ref_counted.addRefCount(), 0UL);
        EXPECT_EQ(ref_counted.refCount(), 1UL);
    }

    EXPECT_EQ(ref_counted.releaseCount(), 1UL);
    EXPECT_EQ(ref_counted.refCount(), 0UL);
}

TEST(Interface, DetachReleasesWrapperOwnership)
{
    RefCountedInterface ref_counted;
    auto ptr = pirks::platform_windows::Interface<RefCountedInterface>::attach(&ref_counted);

    EXPECT_EQ(ptr.detach(), &ref_counted);
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ref_counted.releaseCount(), 0UL);
    EXPECT_EQ(ref_counted.refCount(), 1UL);
}
