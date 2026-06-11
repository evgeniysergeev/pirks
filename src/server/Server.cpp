#include "Server.h"

#include <spdlog/spdlog.h>

#include <utility>

#include "ControlPlaneServer.h"
#include "TCPConnection.h"
#include "UDPConnection.h"

Server::Server(ServerConfig::ConnectionType connectionType, ControlPlaneConfig controlPlaneConfig)
        : connectionType_ { connectionType }
        , controlPlaneConfig_ { std::move(controlPlaneConfig) }
        , connection_ { nullptr }
{
    //
}

Server::~Server()
{
    //
}

void Server::run()
{
    spdlog::info("Run server");

    inPackets_.reset(new PacketsQueue());
    outPackets_.reset(new PacketsQueue());

    switch (connectionType_) {
    case ServerConfig::ConnectionType::Default:
        [[fallthrough]];
    case ServerConfig::ConnectionType::UDP:
        connection_.reset(new UDPConnection());
        break;

    case ServerConfig::ConnectionType::TCP:
        connection_.reset(new TCPConnection());
        break;
    }

    // Checking that connection was made
    assert(connection_ && "Connection is NULL, but should be already created");

    connection_->create(inPackets_, outPackets_);

    controlPlaneServer_.reset(new ControlPlaneServer(controlPlaneConfig_));
    controlPlaneServer_->start();
    controlPlaneServer_->wait();
}

void Server::stop()
{
    spdlog::info("Stop server");

    if (controlPlaneServer_) {
        controlPlaneServer_->stop();
        controlPlaneServer_.reset();
    }

    connection_.reset();
    inPackets_.reset();
    outPackets_.reset();
}
