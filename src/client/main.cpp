#include <spdlog/spdlog.h>

#include "Client.h"
#include "ClientConfig.h"
#include "ExitCode.h"
#include "StrUtils.h"
#include "deferral.h"
#include "version.h"

int main(int argc, char **argv)
{
    try {
        ClientConfig config;

        const auto ret =
                config.parseArgs(PROJECT_DESCRIPTION, "pirks-client", PROJECT_VERSION, argc, argv);
        if (config.shouldExit()) {
            return ret;
        }

        spdlog::info(
                "{} client v{} (Platform: {})"sv,
                PROJECT_NAME,
                PROJECT_FULL_VERSION,
                PROJECT_PLATFORM);
        defer
        {
            spdlog::info("{} client v{} exited"sv, PROJECT_NAME, PROJECT_VERSION);
        };

        if (config.isDebug()) {
            spdlog::set_level(spdlog::level::debug);
            spdlog::debug("Debug logging is enabled");
        }

        Client client { config };
        client.run();

    } catch (const std::exception &error) {
        spdlog::critical("Exception thrown: {}", error.what());
        return ExitCode::ExceptionThrown;
    } catch (...) {
        spdlog::critical("Unknown exception thrown");
        return ExitCode::ExceptionThrown;
    }

    return ExitCode::OK;
}
