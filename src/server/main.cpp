#include <spdlog/spdlog.h>

#include "str_utils.h"
#include "version.h"

#include <CLI/CLI.hpp>

int main(int argc, char** argv)
{
    // Print version information early
    spdlog::info("{} v{} (Platform: {})"sv, PROJECT_NAME, PROJECT_FULL_VERSION, PROJECT_PLATFORM);

    CLI::App args { PROJECT_NAME };
    argv = args.ensure_utf8(argv);  // new argv memory is held by app

    // TODO: Here your flags / options

    try {
        args.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return args.exit(e);
    }

    return 0;
}
