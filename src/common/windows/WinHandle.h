#pragma once

#include <windows.h>

namespace pirks::platform_windows
{

/**
 * @brief RAII class for WinAPI Handles
 *
 */
class WinHandle final
{
public:
    WinHandle(HANDLE handle = INVALID_HANDLE_VALUE) noexcept : handle_ { handle } {}

    WinHandle(const WinHandle &)            = delete;
    WinHandle &operator=(const WinHandle &) = delete;

    WinHandle(WinHandle &&other) noexcept : handle_ { other.handle_ }
    {
        other.handle_ = INVALID_HANDLE_VALUE;
    }

    WinHandle &operator=(WinHandle &&other) noexcept
    {
        if (this != &other) {
            reset();
            handle_       = other.handle_;
            other.handle_ = INVALID_HANDLE_VALUE;
        }
        return *this;
    }

    ~WinHandle() { reset(); }

public:
    operator bool() const noexcept
    {
        return handle_ != INVALID_HANDLE_VALUE && handle_ != nullptr;
    }

    HANDLE get() const noexcept {
        return handle_;
    }

    HANDLE release() noexcept
    {
        HANDLE tmp = handle_;
        handle_    = INVALID_HANDLE_VALUE;
        return tmp;
    }

    void reset(HANDLE new_handle = INVALID_HANDLE_VALUE) noexcept
    {
        if (handle_ != INVALID_HANDLE_VALUE && handle_ != nullptr) {
            CloseHandle(handle_);
        }
        handle_ = new_handle;
    }

    void swap(WinHandle &other) noexcept { std::swap(handle_, other.handle_); }

private:
    HANDLE handle_;
};

}; // namespace pirks::platform_windows
