/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <audioclient.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

#include "AudioFormats.h"
#include "DeviceEnumeratorPtr.h"
#include "Interface.h"

namespace audio::capture_audio::platform_windows
{

class AudioClientPtr final: public ::pirks::platform_windows::Interface<IAudioClient>
{
public:
    AudioClientPtr(DevicePtr &device, const AudioFormat &format)
    {
        HRESULT status = device->Activate(
                IID_IAudioClient,
                CLSCTX_ALL,
                nullptr,
                reinterpret_cast<LPVOID *>(&pointer_));

        if (FAILED(status)) {
            spd::error("Couldn't create Audio Client. HRESULT = 0x{:X}", status);
            // TODO: std::format here
            throw std::runtime_error("Couldn't create Audio Client");
        }

        WAVEFORMATEXTENSIBLE capture_waveformat = createWaveformat(
                SampleFormat::f32,
                format.channelCount,
                format.captureWaveformatChannelMask);

        WaveFormat mixer_waveformat;
        status = pointer_->GetMixFormat(&mixer_waveformat);
        if (FAILED(status)) {
            spd::error("Couldn't get mix format for audio device. HRESULT = 0x{:X}", status);
            // TODO: std::format here
            throw std::runtime_error("Couldn't get mix format for audio device");
        }

        // Prefer the native channel layout of captured audio device when channel counts match
        if (mixer_waveformat->nChannels == format.channelCount
            && mixer_waveformat->wFormatTag == WAVE_FORMAT_EXTENSIBLE
            && mixer_waveformat->cbSize >= 22)
        {
            auto waveformatext_pointer =
                    reinterpret_cast<const WAVEFORMATEXTENSIBLE *>(mixer_waveformat.get());
            capture_waveformat.dwChannelMask = waveformatext_pointer->dwChannelMask;
        }

        // TODO: fix sytle here. alignment is wrong
        status = pointer_->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_LOOPBACK                       //
                        | AUDCLNT_STREAMFLAGS_EVENTCALLBACK        //
                        | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM       //
                        | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, // Enable automatic resampling to
                                                                   // 48 KHz
                0,
                0,
                reinterpret_cast<LPVOID *>(&capture_waveformat),
                nullptr);

        if (status) {
            spd::error(
                    "Couldn't initialize audio client for {}. HRESULT = 0x{:X}",
                    format.name,
                    status);
            // TODO: std::format here
            throw std::runtime_error("Couldn't initialize audio client");
        }

        spd::info("Audio capture format is {}", waveformatToStr(capture_waveformat));
    }
};

}; // namespace audio::capture_audio::platform_windows
