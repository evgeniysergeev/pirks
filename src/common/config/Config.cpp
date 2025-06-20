#include "Config.h"

namespace pirks::config
{

int Config::parseArgs(
        const std::string &app_description,
        const std::string &app_name,
        int                argc,
        char             **argv)
{
    CLI::App args { app_description, app_name };
    argv = args.ensure_utf8(argv); // new argv memory is held by app

    addOptions(args);

    try {
        args.parse(argc, argv);
        if (isHelp_) {
            throw CLI::CallForHelp();
        }
        parseOptions(args);
    } catch (const CLI::ParseError &e) {
        return args.exit(e);
    }

    return 0;
}

void Config::addOptions([[maybe_unused]] CLI::App &args)
{
    // Using help option is from https://github.com/CLIUtils/CLI11/blob/main/examples/modhelp.cpp example
    // Remove help flag because it shortcuts all processing
    args.set_help_flag();
    // Add custom flag that activates help
    args.add_flag("-h,--help", isHelp_, "Request help");

    args.add_flag("-d,--debug", isDebug_, "Enable debug logging");
}

void Config::parseOptions([[maybe_unused]] CLI::App &args)
{
    // Here your can read flags / options after CLI::parse was made
}

} // namespace pirks::config
