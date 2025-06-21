#include "Server.h"

#include <spdlog/spdlog.h>

#include "TCPConnection.h"

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

    if (connectionType_ == ServerConfig::ConnectionType::TCP) {
        connection_.reset(new TCPConnection());
    }
}

void Server::stop()
{
    spdlog::info("Stop server");

    connection_.reset();
}

}; // namespace pirks
