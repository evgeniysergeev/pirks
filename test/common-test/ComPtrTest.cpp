#include <gtest/gtest.h>

#include <utility>

#include "ComPtr.h"

namespace
{

class RefCountedInterface
{
public:
    auto AddRef() -> ULONG
    {
        addRefCount_++;
        refCount_++;
        return refCount_;
    }

    auto Release() -> ULONG
    {
        releaseCount_++;
        refCount_--;
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

TEST(ComPtr, BorrowedConstructorAddsReference)
{
    RefCountedInterface ref_counted;

    {
        ComPtr<RefCountedInterface> ptr { &ref_counted };

        EXPECT_EQ(ptr.get(), &ref_counted);
        EXPECT_EQ(ref_counted.addRefCount(), 1UL);
        EXPECT_EQ(ref_counted.refCount(), 2UL);
    }

    EXPECT_EQ(ref_counted.releaseCount(), 1UL);
    EXPECT_EQ(ref_counted.refCount(), 1UL);
}

TEST(ComPtr, CopyAddsReference)
{
    RefCountedInterface ref_counted;

    {
        ComPtr<RefCountedInterface> ptr { &ref_counted };

        {
            ComPtr<RefCountedInterface> copy { ptr };

            EXPECT_EQ(copy.get(), &ref_counted);
            EXPECT_EQ(ref_counted.addRefCount(), 2UL);
            EXPECT_EQ(ref_counted.refCount(), 3UL);
        }

        EXPECT_EQ(ref_counted.releaseCount(), 1UL);
        EXPECT_EQ(ref_counted.refCount(), 2UL);
    }

    EXPECT_EQ(ref_counted.releaseCount(), 2UL);
    EXPECT_EQ(ref_counted.refCount(), 1UL);
}

TEST(ComPtr, MoveTransfersReference)
{
    RefCountedInterface ref_counted;

    {
        ComPtr<RefCountedInterface> ptr { &ref_counted };
        ComPtr<RefCountedInterface> moved { std::move(ptr) };

        EXPECT_FALSE(ptr);
        EXPECT_EQ(moved.get(), &ref_counted);
        EXPECT_EQ(ref_counted.addRefCount(), 1UL);
        EXPECT_EQ(ref_counted.refCount(), 2UL);
    }

    EXPECT_EQ(ref_counted.releaseCount(), 1UL);
    EXPECT_EQ(ref_counted.refCount(), 1UL);
}

TEST(ComPtr, ResetAndGetAddressReleasesCurrentPointer)
{
    RefCountedInterface first;
    RefCountedInterface second;

    {
        ComPtr<RefCountedInterface> ptr { &first };

        RefCountedInterface **out = ptr.resetAndGetAddress();
        EXPECT_FALSE(ptr);
        EXPECT_EQ(first.releaseCount(), 1UL);
        EXPECT_EQ(first.refCount(), 1UL);
        EXPECT_EQ(*out, nullptr);

        *out = &second;
        EXPECT_EQ(ptr.get(), &second);
        EXPECT_EQ(second.addRefCount(), 0UL);
        EXPECT_EQ(second.refCount(), 1UL);
    }

    EXPECT_EQ(second.releaseCount(), 1UL);
    EXPECT_EQ(second.refCount(), 0UL);
}
