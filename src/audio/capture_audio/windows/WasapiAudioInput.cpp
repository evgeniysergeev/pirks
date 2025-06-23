/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#include "WasapiAudioInput.h"

#include "AudioFormats.h"

namespace audio::capture_audio::platform_windows
{

WasapiAudioInput::WasapiAudioInput(
        uint8_t                   channels,
        [[maybe_unused]] uint32_t sample_rate,
        [[maybe_unused]] uint32_t frame_size)
{
    audioEvent_ = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    if (audioEvent_ == ERROR_INVALID_HANDLE) {
        throw std::runtime_error("Unable to create Event handle");
    }

    deviceEnumerator_.reset(new DeviceEnumeratorPtr());

    HRESULT status = deviceEnumerator_->RegisterEndpointNotificationCallback(&audioNotification_);
    if (FAILED(status)) {
        spd::error(("Couldn't register endpoint notification. HRESULT = 0x{:X}", status);
        throw std::runtime_error("Couldn't register endpoint notification");
    }

    device_ = deviceEnumerator_->getDefaultDevice();
    if (!device_) {
        throw std::runtime_error("Can't find default device");
    }

    for (const auto &format: s_Formats) {
        if (format.channelCount != channels) {
            spd::debug(
                    "Skipping audio format {} with channel count {} != {}",
                    format.name,
                    format.channelCount,
                    channels);
            continue;
        }

        spd::debug("Trying audio format {}", format.name);
        try {
            audioClient_.reset(new AudioClientPtr(device, format));
        } catch (const std::exception &e) {
            std::error("Exception while trying audio format {}: {}", format.name, e.what());
            continue;
        }

        spd::debug("Found audio format {}", format.name);
        break;
    }

    if (!audioClient_) {
        throw std::runtime_error("Couldn't find supported format for audio");
    }

    REFERENCE_TIME default_latency;
    audioClient_->GetDevicePeriod(&default_latency, nullptr);
    default_latency_ms = default_latency / 1000;

    std::uint32_t frames;
    status = audioClient_->GetBufferSize(&frames);
    if (FAILED(status)) {
        spd::error("Couldn't acquire the number of audio frames. HRESULT = 0x{:X}", status);
        return -1;
    }

    // *2 because needs to fit double
    buffer_    = util::buffer_t<float> { std::max(frames, frame_size) * 2 * channels };
    bufferPos_ = std::begin(sample_buf);

    {
        IAudioCaptureClient *audio_capture = nullptr;
        status = audioClient_->GetService(IID_IAudioCaptureClient, (void **) &audio_capture);
        if (FAILED(status)) {
            BOOST_LOG(error) << "Couldn't initialize audio capture client [0x"sv
                             << util::hex(status).to_string_view() << ']';

            return -1;
        }
        audioCapture_ = audio_capture;
    }

    status = audio_client->SetEventHandle(audio_event.get());
    if (FAILED(status)) {
        BOOST_LOG(error) << "Couldn't set event handle [0x"sv << util::hex(status).to_string_view()
                         << ']';

        return -1;
    }

    {
        DWORD task_index  = 0;
        mmcss_task_handle = AvSetMmThreadCharacteristics("Pro Audio", &task_index);
        if (!mmcss_task_handle) {
            BOOST_LOG(error)
                    << "Couldn't associate audio capture thread with Pro Audio MMCSS task [0x"
                    << util::hex(GetLastError()).to_string_view() << ']';
        }
    }

    status = audio_client->Start();
    if (FAILED(status)) {
        BOOST_LOG(error) << "Couldn't start recording [0x"sv << util::hex(status).to_string_view()
                         << ']';

        return -1;
    }

    return 0;
}

WasapiAudioInput::~WasapiAudioInput() {}

{
    if (device_enum) {
        device_enum->UnregisterEndpointNotificationCallback(&endpt_notification);
    }

    if (audio_client) {
        audio_client->Stop();
    }

    if (mmcss_task_handle) {
        AvRevertMmThreadCharacteristics(mmcss_task_handle);
    }
}

WasapiAudioInput::sample(std::vector<float> &sample_out)->CaptureResult
{
    const auto sample_size64 = sample_out.size();
    assert(sample_size64 < UINT32_MAX);
    const uint32_t sample_size = static_cast<uint32_t>(sample_size64);

    // Refill the sample buffer if needed
    while (sample_buf_pos - std::begin(sample_buf) < sample_size) {
        auto capture_result = _fill_buffer();
        if (capture_result != capture_e::ok) {
            return capture_result;
        }
    }

    // Fill the output buffer with samples
    std::copy_n(std::begin(sample_buf), sample_size, std::begin(sample_out));

    // Move any excess samples to the front of the buffer
    std::move(&sample_buf[sample_size], sample_buf_pos, std::begin(sample_buf));
    sample_buf_pos -= sample_size;

    return capture_e::ok;
}

capture_e fill_buffer()
{
    HRESULT status;

    // Total number of samples
    struct sample_aligned_t
    {
        std::uint32_t uninitialized;
        float        *samples;
    } sample_aligned;

    // number of samples / number of channels
    struct block_aligned_t
    {
        std::uint32_t audio_sample_size;
    } block_aligned;

    // Check if the default audio device has changed
    if (endpt_notification.check_default_render_device_changed()) {
        // Invoke the audio_control_t's callback if it wants one
        if (default_endpt_changed_cb) {
            (*default_endpt_changed_cb)();
        }

        // Reinitialize to pick up the new default device
        return capture_e::reinit;
    }

    status = WaitForSingleObjectEx(audio_event.get(), default_latency_ms, FALSE);
    switch (status) {
    case WAIT_OBJECT_0:
        break;

    case WAIT_TIMEOUT:
        return capture_e::timeout;

    default:
        BOOST_LOG(error) << "Couldn't wait for audio event: [0x"sv
                         << util::hex(status).to_string_view() << ']';
        return capture_e::error;
    }

    std::uint32_t packet_size {};
    for (status = audio_capture->GetNextPacketSize(&packet_size);
         SUCCEEDED(status) && packet_size > 0;
         status = audio_capture->GetNextPacketSize(&packet_size))
    {
        DWORD buffer_flags;
        status = audio_capture->GetBuffer(
                (BYTE **) &sample_aligned.samples,
                &block_aligned.audio_sample_size,
                &buffer_flags,
                nullptr,
                nullptr);

        switch (status) {
        case S_OK:
            break;

        case AUDCLNT_E_DEVICE_INVALIDATED:
            return capture_e::reinit;

        default:
            BOOST_LOG(error) << "Couldn't capture audio [0x"sv << util::hex(status).to_string_view()
                             << ']';
            return capture_e::error;
        }

        if (buffer_flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) {
            BOOST_LOG(debug) << "Audio capture signaled buffer discontinuity";
        }

        sample_aligned.uninitialized = std::end(sample_buf) - sample_buf_pos;
        auto n = std::min(sample_aligned.uninitialized, block_aligned.audio_sample_size * channels);

        if (n < block_aligned.audio_sample_size * channels) {
            BOOST_LOG(warning) << "Audio capture buffer overflow";
        }

        if (buffer_flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            std::fill_n(sample_buf_pos, n, 0);
        } else {
            std::copy_n(sample_aligned.samples, n, sample_buf_pos);
        }

        sample_buf_pos += n;

        audio_capture->ReleaseBuffer(block_aligned.audio_sample_size);
    }

    if (status == AUDCLNT_E_DEVICE_INVALIDATED) {
        return capture_e::reinit;
    }

    if (FAILED(status)) {
        return capture_e::error;
    }

    return capture_e::ok;
}

}; // namespace audio::capture_audio::platform_windows
