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
            int                argc,
            char             **argv);

protected:
    virtual void addOptions(CLI::App &args);
    virtual void parseOptions(CLI::App &args);

public:
    bool isDebug() { return isDebug_; }

private:
    bool isDebug_;
};

}; // namespace pirks::config
