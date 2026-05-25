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
#include "ComPtr.h"
#include "StrUtils.h"
#include "deferral.h"

namespace audio::capture_audio::platform_windows
{

using MmDevice = pirks::platform_windows::ComPtr<IMMDevice>;

class AudioDeviceEnumerator final
{
public:
    AudioDeviceEnumerator()
    {
        HRESULT status = CoCreateInstance(
                CLSID_MMDeviceEnumerator,
                nullptr,
                CLSCTX_ALL,
                IID_IMMDeviceEnumerator,
                reinterpret_cast<LPVOID *>(enumerator_.resetAndGetAddress()));
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format("Couldn't create Device Enumerator. HRESULT = 0x{:X}", status));
        }
    }

    AudioDeviceEnumerator(const AudioDeviceEnumerator &)            = delete;
    AudioDeviceEnumerator &operator=(const AudioDeviceEnumerator &) = delete;
    AudioDeviceEnumerator(AudioDeviceEnumerator &&)                 = delete;
    AudioDeviceEnumerator &operator=(AudioDeviceEnumerator &&)      = delete;

public:
    auto getDefaultDevice() -> MmDevice
    {
        MmDevice device;
        HRESULT  status = enumerator_->GetDefaultAudioEndpoint(
                eCapture,
                eMultimedia,
                device.resetAndGetAddress());

        if (FAILED(status)) {
            // TODO: print status
            return {};
        }

        return device;
    }

    auto getDeviceNames() -> std::vector<std::string>
    {
        pirks::platform_windows::ComPtr<IMMDeviceCollection> collection;
        HRESULT status = enumerator_->EnumAudioEndpoints(
                eCapture,
                DEVICE_STATE_ACTIVE,
                collection.resetAndGetAddress());
        if (FAILED(status) || !collection) {
            return {};
        }

        UINT count = 0;
        status     = collection->GetCount(&count);
        if (FAILED(status)) {
            return {};
        }

        std::vector<std::string> result;
        for (UINT i = 0; i < count; ++i) {
            MmDevice device;
            status = collection->Item(i, device.resetAndGetAddress());
            if (FAILED(status) || !device) {
                continue;
            }

            pirks::platform_windows::ComPtr<IPropertyStore> property_store;
            status = device->OpenPropertyStore(STGM_READ, property_store.resetAndGetAddress());
            if (FAILED(status) || !property_store) {
                continue;
            }

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

    auto getDeviceByName(const std::string &name) -> MmDevice
    {
        pirks::platform_windows::ComPtr<IMMDeviceCollection> collection;
        HRESULT status = enumerator_->EnumAudioEndpoints(
                eCapture,
                DEVICE_STATE_ACTIVE,
                collection.resetAndGetAddress());
        if (FAILED(status) || !collection) {
            return {};
        }

        UINT count = 0;
        status     = collection->GetCount(&count);
        if (FAILED(status)) {
            return {};
        }

        for (UINT i = 0; i < count; ++i) {
            MmDevice device;
            status = collection->Item(i, device.resetAndGetAddress());
            if (FAILED(status) || !device) {
                continue;
            }

            pirks::platform_windows::ComPtr<IPropertyStore> property_store;
            status = device->OpenPropertyStore(STGM_READ, property_store.resetAndGetAddress());
            if (FAILED(status) || !property_store) {
                continue;
            }

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
                    return device;
                }
            }
        }

        return {};
    }

    void registerEndpointNotificationCallback(IMMNotificationClient *notification_client)
    {
        const HRESULT status =
                enumerator_->RegisterEndpointNotificationCallback(notification_client);
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't register endpoint notification. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }
    }

    void unregisterEndpointNotificationCallback(IMMNotificationClient *notification_client) noexcept
    {
        enumerator_->UnregisterEndpointNotificationCallback(notification_client);
    }

private:
    pirks::platform_windows::ComPtr<IMMDeviceEnumerator> enumerator_;
};

}; // namespace audio::capture_audio::platform_windows
