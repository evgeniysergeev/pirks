#include "Config.h"

#include "ExitCode.h"

namespace pirks::config
{

int Config::parseArgs(
        const std::string &app_description,
        const std::string &app_name,
        const std::string &version,
        int                argc,
        char             **argv)
{
    CLI::App args { app_description, app_name };
    argv = args.ensure_utf8(argv); // new argv memory is held by app

    addStandardOptions(args, version);
    addOptions(args);

    try {
        args.parse(argc, argv);
        if (!parseOptions(args)) {
            shouldExit_ = true;
            return ExitCode::ConfigurationError;
        }
    } catch (const CLI::ParseError &e) {
        shouldExit_ = true;
        return args.exit(e);
    }

    return ExitCode::OK;
}

void Config::addStandardOptions(CLI::App &args, const std::string &version)
{
    // add version output
    args.set_version_flag("-v,--version", version);

    args.add_flag("-d,--debug", isDebug_, "Enable debug logging");
    args.add_option("-p,--port", port_, "Server port");
}

void Config::addOptions([[maybe_unused]] CLI::App &args)
{
    // Here you can add your custom flags and options
}

bool Config::parseOptions([[maybe_unused]] CLI::App &args)
{
    // Here your can read flags / options after CLI::parse was made
    return true;
}

} // namespace pirks::config
