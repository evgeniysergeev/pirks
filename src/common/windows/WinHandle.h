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
    WinHandle(HANDLE handle) : handle_ { handle } {}
    ~WinHandle()
    {
        if (handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
    }

public:
    operator bool() const { return handle_ != INVALID_HANDLE_VALUE; }
    HANDLE get() const { return handle_; }

private:
    HANDLE handle_;
};

}; // namespace pirks::platform_windows
