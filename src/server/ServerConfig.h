#pragma once

#include "Config.h"

namespace pirks::config
{

class ServerConfig final: public Config
{
public:
    enum class ConnectionType
    {
        Default = 0, ///< Default (for now default is UDP)
        UDP,         ///< Use UDP for networking
        TCP,         ///< Use TCP/IP for networking
        // TODO: libwebsockets
        // TODO: enet
        // TODO: quic
    };

public:
    auto connectionType() const -> ConnectionType { return connectionType_; }

protected:
    void addOptions(CLI::App &args) override;
    bool parseOptions(CLI::App &args) override;

private:
    ConnectionType connectionType_ { ConnectionType::Default };

    // this members needed only to read config options from command line
    bool isTCP_ { false };
    bool isUDP_ { false };
};

}; // namespace pirks::config
