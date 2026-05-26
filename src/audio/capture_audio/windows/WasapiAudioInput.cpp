/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#include "WasapiAudioInput.h"

#include <avrt.h>
#include <windows.h>

// if defined in windows.h
#undef min
#undef max
#include <spdlog/spdlog.h>

#include <algorithm> // for std::max
#include <cassert>
#include <cstdint>
#include <format>
#include <stdexcept>

#include "AudioFormats.h"
#include "deferral.h"

namespace audio::capture_audio::platform_windows
{

namespace
{

auto createAudioEvent() -> pirks::platform_windows::NullWinHandle
{
    pirks::platform_windows::NullWinHandle audio_event =
            CreateEventA(nullptr, FALSE, FALSE, nullptr);
    if (!audio_event) {
        throw std::runtime_error("Unable to create Event handle");
    }

    return audio_event;
}

auto selectDeviceByName(AudioDeviceEnumerator &enumerator, const std::string &audio_source)
        -> MmDevice
{
    return enumerator.getDeviceByName(audio_source);
}

auto selectRequiredDeviceByName(AudioDeviceEnumerator &enumerator, const std::string &audio_source)
        -> MmDevice
{
    MmDevice device = selectDeviceByName(enumerator, audio_source);
    if (!device) {
        throw std::runtime_error("Can't find audio device: " + audio_source);
    }

    return device;
}

auto createCompatibleWasapiAudioClient(MmDevice &device, uint8_t channels) -> WasapiAudioClient
{
    for (const auto &format: s_AudioFormats) {
        if (format.channelCount != channels) {
            spdlog::debug(
                    "Skipping audio format {} with channel count {} != {}",
                    format.name,
                    format.channelCount,
                    channels);
            continue;
        }

        spdlog::debug("Trying audio format {}", format.name);
        try {
            WasapiAudioClient audio_client { device, format };
            spdlog::debug("Found audio format {}", format.name);
            return audio_client;
        } catch (const std::exception &e) {
            spdlog::warn("Exception while trying audio format {}: {}", format.name, e.what());
            continue;
        }
    }

    throw std::runtime_error("Couldn't find supported format for audio");
}

auto createMmcssTaskHandle() -> MmcssTaskHandle
{
    DWORD           task_index  = 0;
    MmcssTaskHandle task_handle = AvSetMmThreadCharacteristics("Pro Audio", &task_index);
    if (!task_handle) {
        throw std::runtime_error(
                std::format(
                        "Couldn't associate audio capture thread with Pro Audio MMCSS task. GetLastError = 0x{:X}",
                        GetLastError()));
    }

    return task_handle;
}

} // namespace

WasapiAudioInput::WasapiAudioInput(
        uint8_t                   channels,
        [[maybe_unused]] uint32_t sample_rate,
        uint32_t                  frame_size,
        const std::string        &audio_source)
        : audioEvent_ { createAudioEvent() }
        , deviceEnumerator_ {}
        , device_ { selectRequiredDeviceByName(deviceEnumerator_, audio_source) }
        , audioClient_ { createCompatibleWasapiAudioClient(device_, channels) }
        , audioCapture_ { audioClient_.createCaptureClient() }
        , defaultLatency_ { audioClient_.defaultLatencyMs() }
        , buffer_ {}
        , bufferPos_ {}
        , channels_ { channels }
{
    const std::uint32_t frames = audioClient_.bufferFrameCount();

    // *2 because needs to fit double
    const uint32_t buffer_size = std::max(frames, frame_size) * 2 * channels_;
    // TODO: check overflow ?
    spdlog::debug("Audio samples buffer size is {}", buffer_size);
    buffer_.resize(buffer_size);
    bufferPos_ = buffer_.data();

    endpointNotificationRegistration_.registerCallback(
            deviceEnumerator_,
            audioNotification_.notificationClient());

    audioClient_.setEventHandle(audioEvent_.get());
    mmcssTaskHandle_ = createMmcssTaskHandle();
    audioClient_.start();
}

WasapiAudioInput::~WasapiAudioInput() = default;

auto WasapiAudioInput::sample(std::vector<float> &sample_out) -> CaptureResult
{
    const auto sample_size64 = sample_out.size();
    assert(sample_size64 < INT32_MAX && "audio sample buffer overflow");
    const int32_t sample_size = static_cast<int32_t>(sample_size64);

    // Refill the sample buffer if needed
    while ((bufferPos_ - buffer_.data()) < sample_size) {
        auto capture_result = fillBuffer();
        if (capture_result != CaptureResult::OK) {
            return capture_result;
        }
    }

    // Fill the output buffer with samples
    std::copy_n(buffer_.data(), sample_size, sample_out.data());

    // TODO: Why ?
    //       Move any excess samples to the front of the buffer
    std::move(buffer_.data() + sample_size, bufferPos_, buffer_.data());
    bufferPos_ -= sample_size;

    return CaptureResult::OK;
}

auto WasapiAudioInput::fillBuffer() -> CaptureResult
{
    // Total number of samples
    struct sample_aligned_t
    {
        std::uint32_t uninitialized;
        float        *samples;
    } sample_aligned; // TODO: Why Aligned ?

    // number of samples / number of channels
    struct block_aligned_t
    {
        std::uint32_t audio_sample_size;
    } block_aligned; // TODO: Why Aligned ?

    // Check if the default audio device has changed
    if (audioNotification_.checkDefaultDeviceChanged()) {
        // TODO: Invoke the audio_control_t's callback if it wants one
        //       std::optional<std::function<void()>> default_endpt_changed_cb;
        // if (default_endpt_changed_cb) {
        //    (*default_endpt_changed_cb)();
        // }

        // Reinitialize to pick up the new default device
        return CaptureResult::Reinit;
    }

    const DWORD ret = WaitForSingleObjectEx(audioEvent_.get(), defaultLatency_, FALSE);
    switch (ret) {
    case WAIT_OBJECT_0:
        break;

    case WAIT_TIMEOUT:
        return CaptureResult::Timeout;

    default:
        spdlog::error("Couldn't wait for audio event. ret = 0x{:X}", ret);
        return CaptureResult::Error;
    }

    HRESULT  status;
    uint32_t packet_size {};
    for (status = audioCapture_->GetNextPacketSize(&packet_size);
         SUCCEEDED(status) && packet_size > 0;
         status = audioCapture_->GetNextPacketSize(&packet_size))
    {
        DWORD buffer_flags;
        status = audioCapture_->GetBuffer(
                reinterpret_cast<BYTE **>(&sample_aligned.samples),
                &block_aligned.audio_sample_size,
                &buffer_flags,
                nullptr,
                nullptr);

        switch (status) {
        case S_OK:
            break;

        case AUDCLNT_E_DEVICE_INVALIDATED:
            return CaptureResult::Reinit;

        default:
            spdlog::error("Couldn't capture audio. HRESULT = 0x{:X}", status);
            return CaptureResult::Error;
        }
        const auto frames_to_release = block_aligned.audio_sample_size;
        defer
        {
            audioCapture_->ReleaseBuffer(frames_to_release);
        };

        if (buffer_flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) {
            spdlog::debug("Audio capture signaled buffer discontinuity");
        }

        const size_t used_size = static_cast<size_t>(bufferPos_ - buffer_.data());
        assert(buffer_.size() - used_size < UINT32_MAX && "audio buffer overflow");
        sample_aligned.uninitialized = static_cast<uint32_t>(buffer_.size() - used_size);

        auto n = std::min(
                sample_aligned.uninitialized, //
                block_aligned.audio_sample_size * channels_);
        if (n < block_aligned.audio_sample_size * channels_) {
            spdlog::warn("Audio capture buffer overflow");
        }

        if (buffer_flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            std::fill_n(bufferPos_, n, 0.0f);
        } else {
            std::copy_n(sample_aligned.samples, n, bufferPos_);
        }

        bufferPos_ += n;
    }

    if (status == AUDCLNT_E_DEVICE_INVALIDATED) {
        return CaptureResult::Reinit;
    }

    if (FAILED(status)) {
        return CaptureResult::Error;
    }

    return CaptureResult::OK;
}

}; // namespace audio::capture_audio::platform_windows
