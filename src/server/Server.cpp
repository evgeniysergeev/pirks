#include "Server.h"
#include "TCPConnection.h"

namespace pirks
{

using namespace ::pirks::config;
using namespace ::pirks::networking;

Server::Server(ServerConfig::ConnectionType connectionType)
    : m_connectionType { connectionType }
{
    if (m_connectionType == ServerConfig::ConnectionType::TCP) {
        m_connection.reset(new TCPConnection());
    }
}

Server::~Server()
{
    // TODO:
}

}; // namespace pirks::server
