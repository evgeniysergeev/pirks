#pragma once

#include "Config.h"

namespace pirks::config
{

class ServerConfig final : public Config
{
protected:
    void addOptions(CLI::App &args) override;
};

}; // namespace pirks::config
