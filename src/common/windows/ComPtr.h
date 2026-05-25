/*
 * RAII pointer for COM interfaces with proper reference counting.
 */

#pragma once

#include <windows.h>

namespace pirks::platform_windows
{

/**
 * @brief RAII pointer for COM interfaces with proper AddRef/Release semantics
 *
 * Use resetAndGetAddress() for COM out-parameters.
 * Copying increments the reference count (AddRef).
 * Moving transfers ownership without AddRef.
 * Destruction calls Release.
 */
template<typename T>
class ComPtr
{
public:
    ComPtr() = default;

    explicit ComPtr(T *pointer) : pointer_ { pointer }
    {
        if (pointer_) {
            pointer_->AddRef();
        }
    }

    /**
     * Releases the current pointer and returns storage for a COM out-parameter.
     */
    auto resetAndGetAddress() -> T **
    {
        releasePointer();
        return &pointer_;
    }

    ~ComPtr()
    {
        releasePointer();
    }

    // Copy: increment reference count
    ComPtr(const ComPtr &other) : pointer_ { other.pointer_ }
    {
        if (pointer_) {
            pointer_->AddRef();
        }
    }

    ComPtr &operator=(const ComPtr &other)
    {
        if (this != &other) {
            T *new_pointer = other.pointer_;
            if (new_pointer) {
                new_pointer->AddRef();
            }

            releasePointer();
            pointer_ = new_pointer;
        }
        return *this;
    }

    // Move: transfer ownership
    ComPtr(ComPtr &&other) noexcept : pointer_ { other.pointer_ }
    {
        other.pointer_ = nullptr;
    }

    ComPtr &operator=(ComPtr &&other) noexcept
    {
        if (this != &other) {
            releasePointer();
            pointer_       = other.pointer_;
            other.pointer_ = nullptr;
        }
        return *this;
    }

    T *get() const
    {
        return pointer_;
    }

    explicit operator bool() const
    {
        return pointer_ != nullptr;
    }

    T *operator->() const
    {
        return pointer_;
    }

    T &operator*() const
    {
        return *pointer_;
    }

private:
    void releasePointer()
    {
        if (pointer_ != nullptr) {
            pointer_->Release();
            pointer_ = nullptr;
        }
    }

    T *pointer_ { nullptr };
};

}; // namespace pirks::platform_windows
