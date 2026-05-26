#include "PulseAudioInput.h"

#include <pulse/channelmap.h>
#include <pulse/def.h>
#include <pulse/error.h>
#include <pulse/sample.h>
#include <pulse/simple.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>

#include "AppConstants.h"

namespace
{

constexpr auto kPulseAudioRecordStreamName = "pirks-record";

constexpr std::array<pa_channel_position_t, 8> kPositionMapping {
    PA_CHANNEL_POSITION_FRONT_LEFT,   PA_CHANNEL_POSITION_FRONT_RIGHT,
    PA_CHANNEL_POSITION_FRONT_CENTER, PA_CHANNEL_POSITION_LFE,
    PA_CHANNEL_POSITION_REAR_LEFT,    PA_CHANNEL_POSITION_REAR_RIGHT,
    PA_CHANNEL_POSITION_SIDE_LEFT,    PA_CHANNEL_POSITION_SIDE_RIGHT,
};

auto checkedChannelCount(int channels) -> std::uint8_t
{
    if (channels <= 0 || channels > static_cast<int>(PA_CHANNELS_MAX)) {
        throw std::runtime_error("Unsupported PulseAudio channel count");
    }

    return static_cast<std::uint8_t>(channels);
}

auto createSampleSpec(std::uint8_t channels, std::uint32_t sample_rate) -> pa_sample_spec
{
    pa_sample_spec sample_spec {};
    sample_spec.format   = PA_SAMPLE_FLOAT32NE;
    sample_spec.rate     = sample_rate;
    sample_spec.channels = channels;

    if (!pa_sample_spec_valid(&sample_spec)) {
        throw std::runtime_error("Invalid PulseAudio sample specification");
    }

    return sample_spec;
}

auto createDefaultChannelMap(std::uint8_t channels) -> pa_channel_map
{
    pa_channel_map channel_map {};
    if (pa_channel_map_init_auto(&channel_map, channels, PA_CHANNEL_MAP_DEFAULT) == nullptr) {
        throw std::runtime_error("Unable to create default PulseAudio channel map");
    }

    return channel_map;
}

auto createMappedChannelMap(std::uint8_t channels, const std::uint8_t *mapping) -> pa_channel_map
{
    pa_channel_map channel_map {};
    channel_map.channels = channels;

    for (std::uint8_t channel = 0; channel < channels; ++channel) {
        const std::uint8_t mapped_position = mapping[channel];
        const auto         position_index  = static_cast<std::size_t>(mapped_position);
        if (position_index >= kPositionMapping.size()) {
            throw std::runtime_error("Unsupported PulseAudio channel mapping");
        }

        channel_map.map[channel] = kPositionMapping[position_index];
    }

    if (!pa_channel_map_valid(&channel_map)) {
        throw std::runtime_error("Invalid PulseAudio channel map");
    }

    return channel_map;
}

auto createChannelMap(std::uint8_t channels, const std::uint8_t *mapping) -> pa_channel_map
{
    if (mapping == nullptr) {
        return createDefaultChannelMap(channels);
    }

    return createMappedChannelMap(channels, mapping);
}

auto checkedFrameBytes(std::uint32_t frame_size, std::uint8_t channels) -> std::uint32_t
{
    if (frame_size == 0) {
        return std::numeric_limits<std::uint32_t>::max();
    }

    const auto byte_count = static_cast<std::uint64_t>(frame_size) * channels * sizeof(float);
    if (byte_count > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("PulseAudio frame buffer is too large");
    }

    return static_cast<std::uint32_t>(byte_count);
}

auto createBufferAttributes(std::uint32_t frame_size, std::uint8_t channels) -> pa_buffer_attr
{
    constexpr auto default_value = std::numeric_limits<std::uint32_t>::max();

    pa_buffer_attr buffer_attr {};
    buffer_attr.maxlength = default_value;
    buffer_attr.tlength   = default_value;
    buffer_attr.prebuf    = default_value;
    buffer_attr.minreq    = default_value;
    buffer_attr.fragsize  = checkedFrameBytes(frame_size, channels);

    return buffer_attr;
}

auto isReinitError(int status) -> bool
{
    switch (status) {
    case PA_ERR_CONNECTIONREFUSED:
    case PA_ERR_CONNECTIONTERMINATED:
    case PA_ERR_BADSTATE:
    case PA_ERR_NOENTITY:
    case PA_ERR_INVALIDSERVER:
        return true;

    default:
        return false;
    }
}

} // namespace

void PulseAudioSimpleDeleter::operator()(pa_simple *stream) const noexcept
{
    pa_simple_free(stream);
}

PulseAudioInput::PulseAudioInput(
        const std::string  &source_name,
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size,
        const std::uint8_t *mapping)
{
    if (source_name.empty()) {
        throw std::runtime_error("PulseAudio source name is empty");
    }

    const std::uint8_t   channel_count = checkedChannelCount(channels);
    const pa_sample_spec sample_spec   = createSampleSpec(channel_count, sample_rate);
    const pa_channel_map channel_map   = createChannelMap(channel_count, mapping);
    const pa_buffer_attr buffer_attr   = createBufferAttributes(frame_size, channel_count);

    int status = 0;
    stream_.reset(pa_simple_new(
            nullptr,
            pirks::kApplicationId,
            PA_STREAM_RECORD,
            source_name.c_str(),
            kPulseAudioRecordStreamName,
            &sample_spec,
            &channel_map,
            &buffer_attr,
            &status));

    if (!stream_) {
        throw std::runtime_error(std::string("pa_simple_new() failed: ") + pa_strerror(status));
    }
}

PulseAudioInput::~PulseAudioInput() = default;

auto PulseAudioInput::sample(std::vector<float> &sample_out) -> CaptureResult
{
    if (sample_out.empty()) {
        return CaptureResult::OK;
    }

    const auto byte_count = sample_out.size() * sizeof(float);

    int status = 0;
    if (pa_simple_read(stream_.get(), sample_out.data(), byte_count, &status) == 0) {
        return CaptureResult::OK;
    }

    spdlog::error("pa_simple_read() failed: {}", pa_strerror(status));

    if (status == PA_ERR_TIMEOUT) {
        return CaptureResult::Timeout;
    }

    if (isReinitError(status)) {
        return CaptureResult::Reinit;
    }

    return CaptureResult::Error;
}
