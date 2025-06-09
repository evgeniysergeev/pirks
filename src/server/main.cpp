#include <spdlog/spdlog.h>

#include <CLI/CLI.hpp>
#include <deferral.hh>

#include "str_utils.h"
#include "version.h"

int parseArgs(int argc, char **argv)
{
    CLI::App args { PROJECT_NAME };
    argv = args.ensure_utf8(argv); // new argv memory is held by app

    // TODO: Here your flags / options

    try {
        args.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return args.exit(e);
    }

    return 0;
}

int main(int argc, char **argv)
{
    // Print version information early
    spdlog::info("{} v{} (Platform: {})"sv, PROJECT_NAME, PROJECT_FULL_VERSION, PROJECT_PLATFORM);
    defer
    {
        spdlog::info("{} v{} exited"sv, PROJECT_NAME, PROJECT_FULL_VERSION);
    };

    const auto ret = parseArgs(argc, argv);
    if (ret != 0) {
        return ret;
    }

    return 0;
}
