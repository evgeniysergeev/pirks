#pragma once

#include <windows.h>
#include <utility>

namespace pirks::platform_windows
{

/**
 * @brief Generic RAII wrapper for WinAPI-style handles
 *
 * @tparam HandleT        The handle type (e.g. HANDLE, HKEY, SC_HANDLE)
 * @tparam InvalidValueT  A callable `HandleT() noexcept` returning the invalid value
 * @tparam CloserT        A callable `void(HandleT) noexcept` that closes the handle
 */
template <typename HandleT, typename InvalidValueT, typename CloserT>
class UniqueHandle
{
public:
    using handle_type       = HandleT;
    using invalid_value_fn  = InvalidValueT;
    using closer_type       = CloserT;

    constexpr UniqueHandle() noexcept
        : handle_ { invalid_value_fn {}() }
    {
    }

    // non-explicit to be able to write something like:
    // WinHandle event = CreateEventA
    UniqueHandle(handle_type handle) noexcept
        : handle_ { handle }
    {
    }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    UniqueHandle(UniqueHandle&& other) noexcept
        : handle_ { other.release() }
    {
    }

    UniqueHandle& operator=(UniqueHandle&& other) noexcept
    {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }

    ~UniqueHandle() noexcept
    {
        reset();
    }

    explicit operator bool() const noexcept
    {
        return handle_ != invalid_value_fn {}();
    }

    handle_type get() const noexcept
    {
        return handle_;
    }

    handle_type release() noexcept
    {
        handle_type tmp = handle_;
        handle_ = invalid_value_fn {}();
        return tmp;
    }

    void reset(handle_type new_handle = invalid_value_fn {}()) noexcept
    {
        if (handle_ != invalid_value_fn {}()) {
            closer_type {}(handle_);
        }
        handle_ = new_handle;
    }

    void swap(UniqueHandle& other) noexcept
    {
        std::swap(handle_, other.handle_);
    }

private:
    handle_type handle_;
};

struct InvalidHandleValue
{
    constexpr HANDLE operator()() const noexcept
    {
        return INVALID_HANDLE_VALUE;
    }
};

struct CloseHandleDeleter
{
    void operator()(HANDLE handle) const noexcept
    {
        if (handle != INVALID_HANDLE_VALUE && handle != nullptr) {
            ::CloseHandle(handle);
        }
    }
};

/**
 * @brief RAII class for WinAPI Handles
 *
 */
using WinHandle = UniqueHandle<HANDLE, InvalidHandleValue, CloseHandleDeleter>;

}; // namespace pirks::platform_windows
