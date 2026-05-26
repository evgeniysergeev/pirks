#pragma once

#include <memory>

#include "IConnection.h"
#include "ServerConfig.h"

class Server final
{
public:
    explicit Server(ServerConfig::ConnectionType connectionType);
    ~Server();

public:
    void run();
    void stop();

private:
    ServerConfig::ConnectionType              connectionType_;
    std::unique_ptr<IConnection>          connection_;
    std::shared_ptr<PacketsQueue>         inPackets_;
    std::shared_ptr<PacketsQueue>         outPackets_;
};
