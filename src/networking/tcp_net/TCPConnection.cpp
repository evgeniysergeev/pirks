#include "TCPConnection.h"

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

namespace pirks::networking
{

TCPConnection::TCPConnection(): m_stop { false }
{
    spdlog::debug("TCPConnection created");
}

TCPConnection::~TCPConnection()
{
    spdlog::debug("TCPConnection destructor");
}

void TCPConnection::recvThreadFunc(TCPConnection *connection)
{
    while (!connection->m_stop) {
        std::this_thread::sleep_for(100ns);
    }
}

void TCPConnection::sendThreadFunc(TCPConnection *connection)
{
    while (!connection->m_stop) {
        std::this_thread::sleep_for(100ns);
    }
}

}; // namespace pirks::networking
