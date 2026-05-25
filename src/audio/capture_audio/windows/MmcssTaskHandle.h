#pragma once

#include <avrt.h>
#include <windows.h>

#include "WinHandle.h"

namespace audio::capture_audio::platform_windows
{

struct MmcssTaskHandleDeleter
{
    void operator()(HANDLE handle) const noexcept
    {
        AvRevertMmThreadCharacteristics(handle);
    }
};

using MmcssTaskHandle = ::pirks::platform_windows::
        UniqueHandle<HANDLE, ::pirks::platform_windows::NullHandleSentinel, MmcssTaskHandleDeleter>;

}; // namespace audio::capture_audio::platform_windows
