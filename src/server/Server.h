#pragma once

#include "ServerConfig.h"
#include "IConnection.h"

#include <memory>


namespace pirks
{

class Server final
{
public:
    Server(config::ServerConfig::ConnectionType connectionType);
    ~Server();

private:
    config::ServerConfig::ConnectionType m_connectionType;
    std::unique_ptr<networking::IConnection> m_connection;
};

}; // namespace pirks::server
