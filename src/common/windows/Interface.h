/*
 * RAII wrapper for COM interfaces with proper reference counting.
 */

#pragma once

#include <windows.h>

namespace pirks::platform_windows
{

/**
 * @brief RAII class for COM Interfaces with proper AddRef/Release semantics
 *
 * Copying increments the reference count (AddRef).
 * Moving transfers ownership without AddRef.
 * Destruction calls Release.
 */
template<typename T>
class Interface
{
public:
    Interface(T *p = nullptr) : pointer_ { p }
    {
        if (pointer_) {
            pointer_->AddRef();
        }
    }

    virtual ~Interface()
    {
        if (pointer_ != nullptr) {
            pointer_->Release();
        }
    }

public:
    // Copy: increment reference count
    Interface(const Interface &other) : pointer_ { other.pointer_ }
    {
        if (pointer_) {
            pointer_->AddRef();
        }
    }

    Interface &operator=(const Interface &other)
    {
        if (this != &other) {
            // Release old
            if (pointer_) {
                pointer_->Release();
            }
            // Copy new
            pointer_ = other.pointer_;
            if (pointer_) {
                pointer_->AddRef();
            }
        }
        return *this;
    }

    // Move: transfer ownership
    Interface(Interface &&other) noexcept : pointer_ { other.pointer_ }
    {
        other.pointer_ = nullptr;
    }

    Interface &operator=(Interface &&other) noexcept
    {
        if (this != &other) {
            if (pointer_) {
                pointer_->Release();
            }
            pointer_    = other.pointer_;
            other.pointer_ = nullptr;
        }
        return *this;
    }

    T *get() const
    {
        return pointer_;
    }

    operator bool() const
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

protected:
    T *pointer_ { nullptr };
};

}; // namespace pirks::platform_windows
