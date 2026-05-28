#pragma once

#include <memory>

#include "ControlPlaneConfig.h"
#include "IConnection.h"
#include "ServerConfig.h"

class ControlPlaneServer;

class Server final
{
public:
    Server(ServerConfig::ConnectionType connectionType, ControlPlaneConfig controlPlaneConfig);
    ~Server();

public:
    void run();
    void stop();

private:
    ServerConfig::ConnectionType        connectionType_;
    ControlPlaneConfig                  controlPlaneConfig_;
    std::unique_ptr<IConnection>        connection_;
    std::unique_ptr<ControlPlaneServer> controlPlaneServer_;
    std::shared_ptr<PacketsQueue>       inPackets_;
    std::shared_ptr<PacketsQueue>       outPackets_;
};
