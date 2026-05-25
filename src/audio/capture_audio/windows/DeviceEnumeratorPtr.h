/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>

#include <format>
#include <stdexcept>
#include <string>
#include <vector>

#include "Interface.h"
#include "AudioUUIDs.h"

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
            return nullptr;
        }

        return device;
    }

    auto getDeviceNames() -> std::vector<std::string>
    {
        std::vector<std::string> result;

        IMMDeviceCollection *collection = nullptr;
        HRESULT              status     = pointer_->EnumAudioEndpoints(
                eRender,
                DEVICE_STATE_ACTIVE,
                &collection);
        if (FAILED(status) || collection == nullptr) {
            return result;
        }

        UINT count = 0;
        collection->GetCount(&count);

        for (UINT i = 0; i < count; ++i) {
            IMMDevice *device = nullptr;
            status            = collection->Item(i, &device);
            if (FAILED(status) || device == nullptr) {
                continue;
            }

            IPropertyStore *property_store = nullptr;
            status                         = device->OpenPropertyStore(STGM_READ, &property_store);
            if (FAILED(status) || property_store == nullptr) {
                device->Release();
                continue;
            }

            PROPVARIANT friendly_name;
            PropVariantInit(&friendly_name);

            status = property_store->GetValue(PKEY_Device_FriendlyName, &friendly_name);
            if (SUCCEEDED(status) && friendly_name.vt == VT_LPWSTR && friendly_name.pwszVal) {
                const std::wstring_view wname { friendly_name.pwszVal };
                std::string             name(wname.begin(), wname.end());
                result.push_back(std::move(name));
            }

            PropVariantClear(&friendly_name);
            property_store->Release();
            device->Release();
        }

        collection->Release();

        return result;
    }

    auto getDeviceByName(const std::string &name) -> DevicePtr
    {
        IMMDeviceCollection *collection = nullptr;
        HRESULT              status     = pointer_->EnumAudioEndpoints(
                eRender,
                DEVICE_STATE_ACTIVE,
                &collection);
        if (FAILED(status) || collection == nullptr) {
            return nullptr;
        }

        UINT count = 0;
        collection->GetCount(&count);

        for (UINT i = 0; i < count; ++i) {
            IMMDevice *device = nullptr;
            status            = collection->Item(i, &device);
            if (FAILED(status) || device == nullptr) {
                continue;
            }

            IPropertyStore *property_store = nullptr;
            status                         = device->OpenPropertyStore(STGM_READ, &property_store);
            if (FAILED(status) || property_store == nullptr) {
                device->Release();
                continue;
            }

            PROPVARIANT friendly_name;
            PropVariantInit(&friendly_name);

            status = property_store->GetValue(PKEY_Device_FriendlyName, &friendly_name);
            if (SUCCEEDED(status) && friendly_name.vt == VT_LPWSTR && friendly_name.pwszVal) {
                const std::wstring_view wname { friendly_name.pwszVal };
                std::string             current_name(wname.begin(), wname.end());

                if (current_name == name) {
                    PropVariantClear(&friendly_name);
                    property_store->Release();
                    collection->Release();
                    return device;
                }
            }

            PropVariantClear(&friendly_name);
            property_store->Release();
            device->Release();
        }

        collection->Release();
        return nullptr;
    }
};

}; // namespace audio::capture_audio::platform_windows
