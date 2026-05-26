#pragma once

#include <mmdeviceapi.h>

#include <stdexcept>

#include "AudioDeviceEnumerator.h"

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

        device_enumerator.registerEndpointNotificationCallback(notification_client);

        deviceEnumerator_   = &device_enumerator;
        notificationClient_ = notification_client;
    }

    void reset() noexcept
    {
        if (deviceEnumerator_ && notificationClient_) {
            deviceEnumerator_->unregisterEndpointNotificationCallback(notificationClient_);
        }

        deviceEnumerator_   = nullptr;
        notificationClient_ = nullptr;
    }

private:
    AudioDeviceEnumerator *deviceEnumerator_ {};
    IMMNotificationClient *notificationClient_ {};
};
