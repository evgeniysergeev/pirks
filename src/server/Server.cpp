#include "Server.h"

#include <spdlog/spdlog.h>

#include "TCPConnection.h"
#include "UDPConnection.h"

namespace pirks
{

using namespace ::pirks::config;
using namespace ::pirks::networking;

Server::Server(ServerConfig::ConnectionType connectionType)
        : connectionType_ { connectionType }
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

    inPackets_.reset(new networking::PacketsQueue());
    outPackets_.reset(new networking::PacketsQueue());

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
}

void Server::stop()
{
    spdlog::info("Stop server");

    connection_.reset();
    inPackets_.reset();
    outPackets_.reset();
}

}; // namespace pirks
