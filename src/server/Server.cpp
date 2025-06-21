#include "Server.h"

#include <spdlog/spdlog.h>

#include "TCPConnection.h"

namespace pirks
{

using namespace ::pirks::config;
using namespace ::pirks::networking;

Server::Server(ServerConfig::ConnectionType connectionType)
        : m_connectionType { connectionType }
        , m_connection { nullptr }
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

    if (m_connectionType == ServerConfig::ConnectionType::TCP) {
        m_connection.reset(new TCPConnection());
    }
}

void Server::stop()
{
    spdlog::info("Stop server");

    m_connection.reset();
}

}; // namespace pirks
