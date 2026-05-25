/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <audioclient.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <utility>

#include "AudioDeviceEnumerator.h"
#include "AudioFormats.h"
#include "AudioUUIDs.h"
#include "ComPtr.h"
#include "deferral.h"

namespace audio::capture_audio::platform_windows
{
class WasapiAudioClient final
{
public:
    WasapiAudioClient(MmDevice &device, const AudioFormat &format)
    {
        HRESULT status = device->Activate(
                IID_IAudioClient,
                CLSCTX_ALL,
                nullptr,
                reinterpret_cast<LPVOID *>(client_.resetAndGetAddress()));

        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't create Audio Client. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }

        WAVEFORMATEXTENSIBLE capture_waveformat = createWaveformat(
                SampleFormat::f32,
                format.channelCount,
                format.captureWaveformatChannelMask);

        WAVEFORMATEX *mixer_waveformat {};
        status = client_->GetMixFormat(&mixer_waveformat);
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't get mix format for audio device. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }
        defer
        {
            CoTaskMemFree(mixer_waveformat);
        };

        // Prefer the native channel layout of captured audio device when channel counts match
        if (mixer_waveformat->nChannels == format.channelCount
            && mixer_waveformat->wFormatTag == WAVE_FORMAT_EXTENSIBLE
            && mixer_waveformat->cbSize >= 22)
        {
            auto waveformatext_pointer =
                    reinterpret_cast<const WAVEFORMATEXTENSIBLE *>(mixer_waveformat);
            capture_waveformat.dwChannelMask = waveformatext_pointer->dwChannelMask;
        }

        // TODO: fix sytle here. alignment is wrong
        const WAVEFORMATEX *waveformat = &capture_waveformat.Format;
        status                         = client_->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_EVENTCALLBACK            //
                        | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM //
                        | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, // Enable automatic resampling to
                                                                   // 48 KHz
                0,
                0,
                waveformat,
                nullptr);

        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't initialize audio client for {}. HRESULT = 0x{:X}",
                            format.name,
                            static_cast<unsigned long>(status)));
        }

        spdlog::info("Audio capture format is {}", waveformatToStr(capture_waveformat));
    }

    WasapiAudioClient(const WasapiAudioClient &)            = delete;
    WasapiAudioClient &operator=(const WasapiAudioClient &) = delete;

    WasapiAudioClient(WasapiAudioClient &&) noexcept = default;

    WasapiAudioClient &operator=(WasapiAudioClient &&other) noexcept
    {
        if (this != &other) {
            stop();
            client_ = std::move(other.client_);
        }
        return *this;
    }

    ~WasapiAudioClient()
    {
        stop();
    }

    auto defaultLatencyMs() const -> DWORD
    {
        REFERENCE_TIME default_latency {};
        const HRESULT  status = client_->GetDevicePeriod(&default_latency, nullptr);
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't get audio device period. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }

        assert(default_latency < UINT32_MAX && "default latency is too big");
        return static_cast<DWORD>(default_latency / 1000);
    }

    auto bufferFrameCount() const -> std::uint32_t
    {
        std::uint32_t frames {};
        const HRESULT status = client_->GetBufferSize(&frames);
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't acquire the number of audio frames. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }

        return frames;
    }

    auto createCaptureClient() const -> pirks::platform_windows::ComPtr<IAudioCaptureClient>
    {
        pirks::platform_windows::ComPtr<IAudioCaptureClient> audio_capture;
        const HRESULT                                        status = client_->GetService(
                IID_IAudioCaptureClient,
                reinterpret_cast<void **>(audio_capture.resetAndGetAddress()));
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't initialize audio capture client. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }

        return audio_capture;
    }

    void setEventHandle(HANDLE audio_event)
    {
        const HRESULT status = client_->SetEventHandle(audio_event);
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't set event handle. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }
    }

    void start()
    {
        const HRESULT status = client_->Start();
        if (FAILED(status)) {
            throw std::runtime_error(
                    std::format(
                            "Couldn't start recording. HRESULT = 0x{:X}",
                            static_cast<unsigned long>(status)));
        }
    }

private:
    void stop() noexcept
    {
        if (client_) {
            client_->Stop();
        }
    }

private:
    pirks::platform_windows::ComPtr<IAudioClient> client_;
};

}; // namespace audio::capture_audio::platform_windows
