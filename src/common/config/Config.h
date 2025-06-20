#pragma once

#include <CLI/CLI.hpp>

namespace pirks::config
{

/**
 * @brief Config class read options from configuration file and program arguments
 *
 */
class Config
{
public:
    Config() {};
    virtual ~Config() = default;

public:
    int parseArgs(
            const std::string &app_description,
            const std::string &app_name,
            const std::string &version,
            int                argc,
            char             **argv);

protected:
    virtual void addOptions(CLI::App &args);
    virtual void parseOptions(CLI::App &args);

private:
    void addStandardOptions(CLI::App &args, const std::string &version);

public:
    bool isDebug() { return isDebug_; }

    bool shouldExit() { return shouldExit_; }

private:
    bool isDebug_ { false };
    bool shouldExit_ { false };
};

}; // namespace pirks::config
