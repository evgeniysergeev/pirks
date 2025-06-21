#pragma once

#include <memory>

#include "IConnection.h"
#include "ServerConfig.h"

namespace pirks
{

class Server final
{
public:
    Server(config::ServerConfig::ConnectionType connectionType);
    ~Server();

public:
    void run();
    void stop();

private:
    config::ServerConfig::ConnectionType     m_connectionType;
    std::unique_ptr<networking::IConnection> m_connection;
};

}; // namespace pirks
