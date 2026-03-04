/*
 * Simple RAII helper for COM initialization on Windows.
 *
 * Usage example:
 *   static pirks::platform_windows::ComInitializer g_com_initializer;
 *
 * This will call CoInitializeEx(nullptr, COINIT_MULTITHREADED) once at startup
 * (when the translation unit is loaded) and CoUninitialize() automatically
 * when the process is shutting down.
 */

#pragma once

#include <windows.h>
#include <objbase.h>

#include <format>
#include <stdexcept>

namespace pirks::platform_windows
{

class ComInitializer
{
public:
    ComInitializer()
    {
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        if (SUCCEEDED(hr)) {
            shouldUninitialize_ = true;
        } else if (hr == S_FALSE || hr == RPC_E_CHANGED_MODE) {
            // COM уже был инициализирован где-то ещё или в другом режиме.
            // В этом случае не вызываем CoUninitialize() в деструкторе.
            shouldUninitialize_ = false;
        } else {
            throw std::runtime_error(
                    std::format("Couldn't initialize COM. HRESULT = 0x{:X}", hr));
        }
    }

    ~ComInitializer()
    {
        if (shouldUninitialize_) {
            CoUninitialize();
        }
    }

    ComInitializer(const ComInitializer &)            = delete;
    ComInitializer &operator=(const ComInitializer &) = delete;

private:
    bool shouldUninitialize_ { false };
};

} // namespace pirks::platform_windows

