#pragma once

#include "Config.h"

namespace pirks::config
{

class ServerConfig final : public Config
{
public:
    enum ConnectionType
    {
        Default = 0, ///< Default (for now it is equal to TCP)
        TCP          ///< Use TCP/IP for networking
    };

public:
    auto connectionType() -> ConnectionType { return connectionType_; }

protected:
    void addOptions(CLI::App &args) override;
    void parseOptions(CLI::App &args) override;

private:
    ConnectionType connectionType_ { Default };

    bool isTCP_ { false };
};

}; // namespace pirks::config
