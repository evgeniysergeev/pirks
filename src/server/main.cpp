#include <spdlog/spdlog.h>

#include <deferral.hh>

#include "ServerConfig.h"
#include "str_utils.h"
#include "version.h"

int main(int argc, char **argv)
{
    pirks::config::ServerConfig config;

    const auto ret =
            config.parseArgs(PROJECT_DESCRIPTION, PROJECT_NAME, PROJECT_VERSION, argc, argv);
    if (config.shouldExit()) {
        // If we printed help screen or version, then do nothing, log nothing, just exit.
        return ret;
    }

    // Print version information early
    spdlog::info("{} v{} (Platform: {})"sv, PROJECT_NAME, PROJECT_FULL_VERSION, PROJECT_PLATFORM);

    // do not print "exited" if called with --help option
    defer { spdlog::info("{} v{} exited"sv, PROJECT_NAME, PROJECT_VERSION); };

    if (config.isDebug()) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Debug logging is enabled");
    }

    return 0;
}
