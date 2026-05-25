#pragma once

#include <mmdeviceapi.h>

#include <format>
#include <stdexcept>

#include "AudioDeviceEnumerator.h"

namespace audio::capture_audio::platform_windows
{

class EndpointNotificationRegistration final
{
public:
    EndpointNotificationRegistration() = default;

    EndpointNotificationRegistration(
            AudioDeviceEnumerator &device_enumerator,
            IMMNotificationClient *notification_client)
    {
        registerCallback(device_enumerator, notification_client);
    }

    EndpointNotificationRegistration(const EndpointNotificationRegistration &)            = delete;
    EndpointNotificationRegistration &operator=(const EndpointNotificationRegistration &) = delete;

    EndpointNotificationRegistration(EndpointNotificationRegistration &&other) noexcept
            : deviceEnumerator_ { other.deviceEnumerator_ }
            , notificationClient_ { other.notificationClient_ }
    {
        other.deviceEnumerator_   = nullptr;
        other.notificationClient_ = nullptr;
    }

    EndpointNotificationRegistration &operator=(EndpointNotificationRegistration &&other) noexcept
    {
        if (this != &other) {
            reset();
            deviceEnumerator_         = other.deviceEnumerator_;
            notificationClient_       = other.notificationClient_;
            other.deviceEnumerator_   = nullptr;
            other.notificationClient_ = nullptr;
        }
        return *this;
    }

    ~EndpointNotificationRegistration()
    {
        reset();
    }

    void registerCallback(
            AudioDeviceEnumerator &device_enumerator,
            IMMNotificationClient *notification_client)
    {
        reset();

        if (notification_client == nullptr) {
            throw std::runtime_error("Endpoint notification client is null");
        }

        const HRESULT status =
                device_enumerator->RegisterEndpointNotificationCallback(notification_client);
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't register endpoint notification. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }

        deviceEnumerator_   = &device_enumerator;
        notificationClient_ = notification_client;
    }

    void reset() noexcept
    {
        if (deviceEnumerator_ && notificationClient_) {
            deviceEnumerator_->get()->UnregisterEndpointNotificationCallback(notificationClient_);
        }

        deviceEnumerator_   = nullptr;
        notificationClient_ = nullptr;
    }

private:
    AudioDeviceEnumerator *deviceEnumerator_ {};
    IMMNotificationClient *notificationClient_ {};
};

}; // namespace audio::capture_audio::platform_windows
