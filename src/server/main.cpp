#include <spdlog/spdlog.h>

#include <deferral.hh>

#include "Config.h"
#include "str_utils.h"
#include "version.h"

int main(int argc, char **argv)
{
    // Print version information early
    spdlog::info("{} v{} (Platform: {})"sv, PROJECT_NAME, PROJECT_FULL_VERSION, PROJECT_PLATFORM);

    pirks::config::Config config;
    const auto ret = config.parseArgs(PROJECT_DESCRIPTION, PROJECT_NAME, argc, argv);
    if (ret != 0) {
        return ret;
    }

    if (config.isHelp()) {
        // If we printed help screen, then do nothing, log nothing, just exit.
        return 0;
    }

    // do not print "exited" if called with --help option
    defer { spdlog::info("{} v{} exited"sv, PROJECT_NAME, PROJECT_FULL_VERSION); };

    if (config.isDebug()) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Debug logging is enabled");
    }

    return 0;
}
