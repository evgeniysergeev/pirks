/*
 * This file is based on src/platform/windows/audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#include <array>
#include <string>
#include <vector>

#include "mmreg.h"

namespace audio::capture_audio::platform_windows
{

constexpr auto SAMPLE_RATE = 48000;

constexpr auto WF_MASK_STEREO = //
        SPEAKER_FRONT_LEFT      //
        | SPEAKER_FRONT_RIGHT;

constexpr auto WF_MASK_SURROUND51_WITH_BACKSPEAKERS = //
        SPEAKER_FRONT_LEFT                            //
        | SPEAKER_FRONT_RIGHT                         //
        | SPEAKER_FRONT_CENTER                        //
        | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT   //
        | SPEAKER_BACK_RIGHT;

constexpr auto WF_MASK_SURROUND51_WITH_SIDESPEAKERS = //
        SPEAKER_FRONT_LEFT                            //
        | SPEAKER_FRONT_RIGHT                         //
        | SPEAKER_FRONT_CENTER                        //
        | SPEAKER_LOW_FREQUENCY                       //
        | SPEAKER_SIDE_LEFT                           //
        | SPEAKER_SIDE_RIGHT;

constexpr auto WF_MASK_SURROUND71 = //
        SPEAKER_FRONT_LEFT          //
        | SPEAKER_FRONT_RIGHT       //
        | SPEAKER_FRONT_CENTER      //
        | SPEAKER_LOW_FREQUENCY     //
        | SPEAKER_BACK_LEFT         //
        | SPEAKER_BACK_RIGHT        //
        | SPEAKER_SIDE_LEFT         //
        | SPEAKER_SIDE_RIGHT;

enum class SampleFormat
{
    f32,
    s32,
    s24in32,
    s24,
    s16,
};

constexpr WAVEFORMATEXTENSIBLE createWaveformat(
        SampleFormat sample_format,
        WORD         channel_count,
        DWORD        channel_mask)
{
    WAVEFORMATEXTENSIBLE waveformat = {};

    switch (sample_format) {
    case SampleFormat::f32:
        waveformat.SubFormat                   = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        waveformat.Format.wBitsPerSample       = 32;
        waveformat.Samples.wValidBitsPerSample = 32;
        break;

    case SampleFormat::s32:
        waveformat.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;
        waveformat.Format.wBitsPerSample       = 32;
        waveformat.Samples.wValidBitsPerSample = 32;
        break;

    case SampleFormat::s24in32:
        waveformat.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;
        waveformat.Format.wBitsPerSample       = 32;
        waveformat.Samples.wValidBitsPerSample = 24;
        break;

    case SampleFormat::s24:
        waveformat.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;
        waveformat.Format.wBitsPerSample       = 24;
        waveformat.Samples.wValidBitsPerSample = 24;
        break;

    case SampleFormat::s16:
        waveformat.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;
        waveformat.Format.wBitsPerSample       = 16;
        waveformat.Samples.wValidBitsPerSample = 16;
        break;

    default:
        static_assert(false, "Unrecognized SampleFormat");
    }

    waveformat.Format.wFormatTag     = WAVE_FORMAT_EXTENSIBLE;
    waveformat.Format.nChannels      = channel_count;
    waveformat.Format.nSamplesPerSec = SAMPLE_RATE;

    waveformat.Format.nBlockAlign =
            waveformat.Format.nChannels * waveformat.Format.wBitsPerSample / 8;
    waveformat.Format.nAvgBytesPerSec =
            waveformat.Format.nSamplesPerSec * waveformat.Format.nBlockAlign;
    waveformat.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

    waveformat.dwChannelMask = channel_mask;

    return waveformat;
}

using VirtualSinkWaveformats = std::vector<WAVEFORMATEXTENSIBLE>;

/**
 * @brief List of supported waveformats for an N-channel virtual audio device
 * @tparam channel_count Number of virtual audio channels
 * @returns std::vector<WAVEFORMATEXTENSIBLE>
 * @note The list of virtual formats returned are sorted in preference order and the first valid
 *       format will be used. All bits-per-sample options are listed because we try to match
 *       this to the default audio device. See also: set_format() below.
 */
template<uint8_t channel_count>
VirtualSinkWaveformats createVirtualSinkFormats()
{
    if constexpr (channel_count == 2) {
        auto channel_mask = waveformat_mask_stereo;
        // The 32-bit formats are a lower priority for stereo because using one will disable
        // Dolby/DTS spatial audio mode if the user enabled it on the Steam speaker.
        return {
            create_waveformat(sample_format_e::s24in32, channel_count, channel_mask),
            create_waveformat(sample_format_e::s24, channel_count, channel_mask),
            create_waveformat(sample_format_e::s16, channel_count, channel_mask),
            create_waveformat(sample_format_e::f32, channel_count, channel_mask),
            create_waveformat(sample_format_e::s32, channel_count, channel_mask),
        };
    } else if (channel_count == 6) {
        auto channel_mask1 = waveformat_mask_surround51_with_backspeakers;
        auto channel_mask2 = waveformat_mask_surround51_with_sidespeakers;
        return {
            create_waveformat(sample_format_e::f32, channel_count, channel_mask1),
            create_waveformat(sample_format_e::f32, channel_count, channel_mask2),
            create_waveformat(sample_format_e::s32, channel_count, channel_mask1),
            create_waveformat(sample_format_e::s32, channel_count, channel_mask2),
            create_waveformat(sample_format_e::s24in32, channel_count, channel_mask1),
            create_waveformat(sample_format_e::s24in32, channel_count, channel_mask2),
            create_waveformat(sample_format_e::s24, channel_count, channel_mask1),
            create_waveformat(sample_format_e::s24, channel_count, channel_mask2),
            create_waveformat(sample_format_e::s16, channel_count, channel_mask1),
            create_waveformat(sample_format_e::s16, channel_count, channel_mask2),
        };
    } else if (channel_count == 8) {
        auto channel_mask = waveformat_mask_surround71;
        return {
            create_waveformat(sample_format_e::f32, channel_count, channel_mask),
            create_waveformat(sample_format_e::s32, channel_count, channel_mask),
            create_waveformat(sample_format_e::s24in32, channel_count, channel_mask),
            create_waveformat(sample_format_e::s24, channel_count, channel_mask),
            create_waveformat(sample_format_e::s16, channel_count, channel_mask),
        };
    }
}

std::string waveformatToStr(const WAVEFORMATEXTENSIBLE &waveformat)
{
    std::string result = //
            waveformat.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ? "F"
            : waveformat.SubFormat == KSDATAFORMAT_SUBTYPE_PCM      ? "S"
                                                                    : "UNKNOWN";

    result += std::to_string(waveformat.Samples.wValidBitsPerSample) + " "
              + std::to_string(waveformat.Format.nSamplesPerSec) + " ";

    switch (waveformat.dwChannelMask) {
    case (waveformat_mask_stereo):
        result += "2.0";
        break;

    case (waveformat_mask_surround51_with_backspeakers):
        result += "5.1";
        break;

    case (waveformat_mask_surround51_with_sidespeakers):
        result += "5.1 (sidespeakers)";
        break;

    case (waveformat_mask_surround71):
        result += "7.1";
        break;

    default:
        result += std::to_string(waveformat.Format.nChannels) + " channels (unrecognized)";
        break;
    }

    return result;
}

struct AudioFormat
{
    uint8_t                channelCount;
    std::string            name;
    int                    captureWaveformatChannelMask;
    VirtualSinkWaveformats virtualSinkWaveformats;
};

static const std::array<const Format, 3> s_AudioFormats = {
    { 2,       "Stereo",                       WF_MASK_STEREO, createVirtualSinkFormats<2>() },
    { 6, "Surround 5.1", WF_MASK_SURROUND51_WITH_BACKSPEAKERS, createVirtualSinkFormats<6>() },
    { 8, "Surround 7.1",                   WF_MASK_SURROUND71, createVirtualSinkFormats<8>() },
};

} // namespace audio::capture_audio::platform_windows
