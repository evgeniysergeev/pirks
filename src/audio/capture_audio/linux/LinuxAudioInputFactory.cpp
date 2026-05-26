#include "LinuxAudioInputFactory.h"

#include <pulse/context.h>
#include <pulse/def.h>
#include <pulse/error.h>
#include <pulse/mainloop.h>
#include <pulse/operation.h>
#include <pulse/pulseaudio.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <exception>
#include <string>
#include <utility>

#include "AppConstants.h"
#include "AudioInputConstants.h"
#include "PulseAudioInput.h"

namespace audio::capture_audio::platform_linux
{

namespace
{

struct PulseMainloopDeleter final
{
    void operator()(pa_mainloop *mainloop) const noexcept
    {
        pa_mainloop_free(mainloop);
    }
};

struct PulseContextDeleter final
{
    void operator()(pa_context *context) const noexcept
    {
        const pa_context_state_t state = pa_context_get_state(context);
        if (state != PA_CONTEXT_UNCONNECTED //
            && state != PA_CONTEXT_FAILED //
            && state != PA_CONTEXT_TERMINATED)
        {
            pa_context_disconnect(context);
        }

        pa_context_unref(context);
    }
};

struct PulseOperationDeleter final
{
    void operator()(pa_operation *operation) const noexcept
    {
        pa_operation_unref(operation);
    }
};

using PulseMainloop  = std::unique_ptr<pa_mainloop, PulseMainloopDeleter>;
using PulseContext   = std::unique_ptr<pa_context, PulseContextDeleter>;
using PulseOperation = std::unique_ptr<pa_operation, PulseOperationDeleter>;

struct ContextState final
{
    bool done {};
    bool ready {};
};

struct SourceListState final
{
    std::vector<std::string> sources;
    bool                     done {};
    bool                     failed {};
};

void contextStateCallback(pa_context *context, void *userdata)
{
    auto *context_state = static_cast<ContextState *>(userdata);

    switch (pa_context_get_state(context)) {
    case PA_CONTEXT_READY:
        context_state->ready = true;
        context_state->done  = true;
        break;

    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        context_state->done = true;
        break;

    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    }
}

void sourceInfoCallback(
        pa_context           *context,
        const pa_source_info *source_info,
        int                   eol,
        void                 *userdata)
{
    auto *source_list_state = static_cast<SourceListState *>(userdata);

    if (eol < 0) {
        spdlog::error(
                "Couldn't get PulseAudio source info: {}",
                pa_strerror(pa_context_errno(context)));
        source_list_state->failed = true;
        source_list_state->done   = true;
        return;
    }

    if (eol > 0) {
        source_list_state->done = true;
        return;
    }

    if (source_info == nullptr || source_info->name == nullptr) {
        return;
    }

    source_list_state->sources.emplace_back(source_info->name);
}

auto waitForContextReady(pa_mainloop *mainloop, pa_context *context) -> bool
{
    ContextState context_state;
    pa_context_set_state_callback(context, contextStateCallback, &context_state);

    while (!context_state.done) {
        int       return_code = 0;
        const int status      = pa_mainloop_iterate(mainloop, 1, &return_code);
        if (status < 0) {
            spdlog::error("PulseAudio mainloop iteration failed");
            return false;
        }
    }

    if (!context_state.ready) {
        spdlog::error("Couldn't connect to PulseAudio: {}", pa_strerror(pa_context_errno(context)));
        return false;
    }

    return true;
}

auto enumeratePulseAudioSources() -> std::vector<std::string>
{
    PulseMainloop mainloop { pa_mainloop_new() };
    if (!mainloop) {
        spdlog::error("Couldn't create PulseAudio mainloop");
        return {};
    }

    PulseContext context {
        pa_context_new(pa_mainloop_get_api(mainloop.get()), pirks::kApplicationId)
    };
    if (!context) {
        spdlog::error("Couldn't create PulseAudio context");
        return {};
    }

    const int connect_status =
            pa_context_connect(context.get(), nullptr, PA_CONTEXT_NOFLAGS, nullptr);
    if (connect_status < 0) {
        spdlog::error(
                "Couldn't connect to PulseAudio: {}",
                pa_strerror(pa_context_errno(context.get())));
        return {};
    }

    if (!waitForContextReady(mainloop.get(), context.get())) {
        return {};
    }

    SourceListState source_list_state;
    PulseOperation  operation {
        pa_context_get_source_info_list(context.get(), sourceInfoCallback, &source_list_state),
    };

    if (!operation) {
        spdlog::error(
                "Couldn't create PulseAudio source info operation: {}",
                pa_strerror(pa_context_errno(context.get())));
        return {};
    }

    while (!source_list_state.done) {
        int       return_code = 0;
        const int status      = pa_mainloop_iterate(mainloop.get(), 1, &return_code);
        if (status < 0) {
            spdlog::error("PulseAudio mainloop iteration failed");
            return {};
        }

        if (pa_operation_get_state(operation.get()) == PA_OPERATION_CANCELLED) {
            return {};
        }
    }

    if (source_list_state.failed) {
        return {};
    }

    return std::move(source_list_state.sources);
}

void appendUnique(std::vector<std::string> &target, std::vector<std::string> &&values)
{
    for (auto &&value: values) {
        if (std::find(target.begin(), target.end(), value) == target.end()) {
            target.push_back(std::move(value));
        }
    }
}

} // namespace

auto LinuxAudioInputFactory::getAudioSources() -> std::vector<std::string>
{
    std::vector<std::string> result { kDefaultAudioSource };
    appendUnique(result, enumeratePulseAudioSources());

    return result;
}

auto LinuxAudioInputFactory::create(
        const std::string  &audio_source,
        int                 channels,
        std::uint32_t       sample_rate,
        std::uint32_t       frame_size,
        const std::uint8_t *mapping) -> std::unique_ptr<IAudioInput>
{
    try {
        return std::make_unique<PulseAudioInput>(
                audio_source,
                channels,
                sample_rate,
                frame_size,
                mapping);
    } catch (const std::exception &e) {
        spdlog::warn("Couldn't create PulseAudio input for [{}]: {}", audio_source, e.what());
        return nullptr;
    }
}

}; // namespace audio::capture_audio::platform_linux
