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
 * COM methods usually return out-parameter interface pointers with a reference
 * already owned by the caller. Use attach() for those pointers.
 * detach() releases wrapper ownership without calling Release().
 * Copying increments the reference count (AddRef).
 * Moving transfers ownership without AddRef.
 * Destruction calls Release.
 */
template<typename T>
class Interface
{
public:
    Interface() = default;

    explicit Interface(T *p) : pointer_ { p }
    {
        if (pointer_) {
            pointer_->AddRef();
        }
    }

    static auto attach(T *p) noexcept -> Interface
    {
        Interface result;
        result.pointer_ = p;
        return result;
    }

    auto detach() noexcept -> T *
    {
        T *detached = pointer_;
        pointer_    = nullptr;
        return detached;
    }

    virtual ~Interface()
    {
        releasePointer();
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
    Interface(Interface &&other) noexcept : pointer_ { other.pointer_ }
    {
        other.pointer_ = nullptr;
    }

    Interface &operator=(Interface &&other) noexcept
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
