#pragma once

#include <memory>

#include "IConnection.h"
#include "ServerConfig.h"

namespace pirks
{

class Server final
{
public:
    explicit Server(config::ServerConfig::ConnectionType connectionType);
    ~Server();

public:
    void run();
    void stop();

private:
    config::ServerConfig::ConnectionType      connectionType_;
    std::unique_ptr<networking::IConnection>  connection_;
    std::shared_ptr<networking::PacketsQueue> inPackets_;
    std::shared_ptr<networking::PacketsQueue> outPackets_;
};

}; // namespace pirks
