/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

// clang-format off
#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
// clang-format on

#include <format>
#include <stdexcept>
#include <string>
#include <vector>

#include "AudioUUIDs.h"
#include "Interface.h"
#include "StrUtils.h"
#include "deferral.h"

namespace audio::capture_audio::platform_windows
{

using DevicePtr = pirks::platform_windows::Interface<IMMDevice>;

class AudioDeviceEnumerator final: public pirks::platform_windows::Interface<IMMDeviceEnumerator>
{
public:
    AudioDeviceEnumerator()
    {
        HRESULT status = CoCreateInstance(
                CLSID_MMDeviceEnumerator,
                nullptr,
                CLSCTX_ALL,
                IID_IMMDeviceEnumerator,
                reinterpret_cast<LPVOID *>(&pointer_));
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format("Couldn't create Device Enumerator. HRESULT = 0x{:X}", status));
        }
    }

public:
    auto getDefaultDevice() -> DevicePtr
    {
        IMMDevice *device = nullptr;
        HRESULT    status = pointer_->GetDefaultAudioEndpoint(eCapture, eMultimedia, &device);

        if (FAILED(status)) {
            // TODO: print status
            return {};
        }

        return DevicePtr::attach(device);
    }

    auto getDeviceNames() -> std::vector<std::string>
    {
        IMMDeviceCollection *collection = nullptr;
        HRESULT status = pointer_->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &collection);
        if (FAILED(status) || collection == nullptr) {
            return {};
        }
        defer
        {
            collection->Release();
        };

        UINT count = 0;
        status     = collection->GetCount(&count);
        if (FAILED(status)) {
            return {};
        }

        std::vector<std::string> result;
        for (UINT i = 0; i < count; ++i) {
            IMMDevice *device = nullptr;
            status            = collection->Item(i, &device);
            if (FAILED(status) || device == nullptr) {
                continue;
            }
            defer
            {
                device->Release();
            };

            IPropertyStore *property_store = nullptr;
            status                         = device->OpenPropertyStore(STGM_READ, &property_store);
            if (FAILED(status) || property_store == nullptr) {
                continue;
            }
            defer
            {
                property_store->Release();
            };

            PROPVARIANT friendly_name;
            PropVariantInit(&friendly_name);
            defer
            {
                PropVariantClear(&friendly_name);
            };

            status = property_store->GetValue(PKEY_Device_FriendlyName, &friendly_name);
            if (SUCCEEDED(status) && friendly_name.vt == VT_LPWSTR && friendly_name.pwszVal) {
                const std::wstring wname { friendly_name.pwszVal };
                result.push_back(wideToUtf8(wname));
            }
        }

        return result;
    }

    auto getDeviceByName(const std::string &name) -> DevicePtr
    {
        IMMDeviceCollection *collection = nullptr;
        HRESULT status = pointer_->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &collection);
        if (FAILED(status) || collection == nullptr) {
            return {};
        }
        defer
        {
            collection->Release();
        };

        UINT count = 0;
        status     = collection->GetCount(&count);
        if (FAILED(status)) {
            return {};
        }

        for (UINT i = 0; i < count; ++i) {
            IMMDevice *device = nullptr;
            status            = collection->Item(i, &device);
            if (FAILED(status) || device == nullptr) {
                continue;
            }
            defer_(device_release)
            {
                device->Release();
            };

            IPropertyStore *property_store = nullptr;
            status                         = device->OpenPropertyStore(STGM_READ, &property_store);
            if (FAILED(status) || property_store == nullptr) {
                continue;
            }
            defer
            {
                property_store->Release();
            };

            PROPVARIANT friendly_name;
            PropVariantInit(&friendly_name);
            defer
            {
                PropVariantClear(&friendly_name);
            };

            status = property_store->GetValue(PKEY_Device_FriendlyName, &friendly_name);
            if (SUCCEEDED(status) && friendly_name.vt == VT_LPWSTR && friendly_name.pwszVal) {
                const std::wstring wname { friendly_name.pwszVal };
                const std::string  current_name = wideToUtf8(wname);

                if (current_name == name) {
                    device_release.release();
                    return DevicePtr::attach(device);
                }
            }
        }

        return {};
    }
};

}; // namespace audio::capture_audio::platform_windows
