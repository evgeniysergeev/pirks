/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <mmdeviceapi.h>

#include <atomic>

#include "WinHandle.h"

namespace audio::capture_audio::platform_windows
{

class AudioNotificationImpl final: public IMMNotificationClient
{
public:
    AudioNotificationImpl() {}
    ~AudioNotificationImpl() {};

    // IUnknown implementation (unused by IMMDeviceEnumerator)
public:
    ULONG STDMETHODCALLTYPE AddRef()
    {
        // TODO: should add reference count
        return 1;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        // TODO: should substract reference count
        return 1;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
    {
        if (IID_IUnknown == riid) {
            AddRef();
            *ppvInterface = static_cast<IUnknown *>(this);
            return S_OK;
        } else if (__uuidof(IMMNotificationClient) == riid) {
            AddRef();
            *ppvInterface = static_cast<IMMNotificationClient *>(this);
            return S_OK;
        } else {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
    }

    // IMMNotificationClient implementation
public:
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
            EDataFlow                flow,
            [[maybe_unused]] ERole   role,
            [[maybe_unused]] LPCWSTR pwstrDeviceId)
    {
        if (flow == eRender) {
            defaultDeviceChanged_.store(true);
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded( //
            [[maybe_unused]] LPCWSTR pwstrDeviceId)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved( //
            [[maybe_unused]] LPCWSTR pwstrDeviceId)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged( //
            [[maybe_unused]] LPCWSTR pwstrDeviceId,
            [[maybe_unused]] DWORD   dwNewState)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
            [[maybe_unused]] LPCWSTR           pwstrDeviceId,
            [[maybe_unused]] const PROPERTYKEY key)
    {
        return S_OK;
    }

    /**
     * @brief Checks if the default rendering device changed and resets the change flag
     * @return `true` if the device changed since last call
     */
    bool checkDefaultDeviceChanged()
    {
        //
        return defaultDeviceChanged_.exchange(false);
    }

private:
    std::atomic_bool defaultDeviceChanged_;
};

}; // namespace audio::capture_audio::platform_windows
