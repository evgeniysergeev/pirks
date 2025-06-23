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
    DeviceEnumeratorPtr()
    {
        HRESULT status = CoCreateInstance(
                CLSID_MMDeviceEnumerator,
                nullptr,
                CLSCTX_ALL,
                IID_IMMDeviceEnumerator,
                reinterpret_cast<LPVOID *>(&pointer_));
        if (FAILED(status)) {
            spdlog::error("Couldn't create Device Enumerator. HRESULT = 0x{:X}", status);
            throw std::runtime_error("Couldn't create Device Enumerator");
        }
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
