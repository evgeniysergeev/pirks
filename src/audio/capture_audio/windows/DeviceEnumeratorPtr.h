/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <mmdeviceapi.h>

#include <stdexcept>

#include "Interface.h"

namespace audio::capture_audio::platform_windows
{

using DevicePtr = pirks::platform_windows::Interface<IMMDevice>;

class DeviceEnumeratorPtr final: public pirks::platform_windows::Interface<IMMDeviceEnumerator>
{
public:
    bool init()
    {
        HRESULT status = CoCreateInstance(
                CLSID_MMDeviceEnumerator,
                nullptr,
                CLSCTX_ALL,
                IID_IMMDeviceEnumerator,
                reinterpret_cast<LPVOID *>(&pointer_));
        if (FAILED(status)) {
            spd::error(("Couldn't create Device Enumerator. HRESUL = 0x{:X}", status);
            return false;
        }

        return true;
    }

public:
    auto getDefaultDevice() -> DevicePtr
    {
        IMMDevice *device = nullptr;
        HRESULT    status = pointer_->GetDefaultAudioEndpoint(eRender, eConsole, &device);

        if (FAILED(status)) {
            // TODO: print status
            return nullptr;
        }

        return device;
    }
};

}; // namespace audio::capture_audio::platform_windows
