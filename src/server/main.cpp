#include <spdlog/spdlog.h>

#include <deferral.hh>

#include "ExitCode.h"
#include "Server.h"
#include "ServerConfig.h"
#include "str_utils.h"
#include "version.h"

int main(int argc, char **argv)
{
    using namespace ::pirks;
    using namespace ::pirks::config;

    try {
        ServerConfig config;

        const auto ret = config.parseArgs( //
                PROJECT_DESCRIPTION,
                PROJECT_NAME,
                PROJECT_VERSION,
                argc, argv);
        if (config.shouldExit()) {
            // If we printed help screen or version, then do nothing, log nothing, just exit.
            return ret;
        }

        // Print version information early
        spdlog::info(
                "{} v{} (Platform: {})"sv,
                PROJECT_NAME,
                PROJECT_FULL_VERSION,
                PROJECT_PLATFORM);
        defer { spdlog::info("{} v{} exited"sv, PROJECT_NAME, PROJECT_VERSION); };

        if (config.isDebug()) {
            spdlog::set_level(spdlog::level::debug);
            spdlog::debug("Debug logging is enabled");
        }

        switch (config.connectionType()) {
        case ServerConfig::ConnectionType::Default:
            [[fallthrough]];
        case ServerConfig::ConnectionType::UDP:
            spdlog::info("Connection type: UDP, port number: {}", config.port());
            break;
        case ServerConfig::ConnectionType::TCP:
            spdlog::info("Connection type: TCP, port number: {}", config.port());
            spdlog::critical("TCP connection is not implemented");
            return ExitCode::ConfigurationError;
        }

        Server server { config.connectionType() };
        server.run();

    } catch (std::exception &e) {
        spdlog::critical("Exception thrown: {}", e.what());
        return ExitCode::ExceptionThrown;
    } catch (...) {
        spdlog::critical("Unknown exception thrown");
        return ExitCode::ExceptionThrown;
    }

    return ExitCode::OK;
}
