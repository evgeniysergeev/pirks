#pragma once

#include <avrt.h>
#include <windows.h>

#include "WinHandle.h"

struct MmcssTaskHandleDeleter
{
    void operator()(HANDLE handle) const noexcept
    {
        AvRevertMmThreadCharacteristics(handle);
    }
};

using MmcssTaskHandle = UniqueHandle<HANDLE, ::NullHandleSentinel, MmcssTaskHandleDeleter>;
