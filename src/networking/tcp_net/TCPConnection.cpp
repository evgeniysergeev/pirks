#include "TCPConnection.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace pirks::networking
{

TCPConnection::TCPConnection() : stop_ { false }
{
    // This comment needed for clang-format. Without it, this will be in one line
    spdlog::debug("TCPConnection created");
}

TCPConnection::~TCPConnection()
{
    spdlog::debug("TCPConnection destructor");

    stop_ = true;
}

void TCPConnection::create(
        std::shared_ptr<PacketsQueue> in_packets,
        std::shared_ptr<PacketsQueue> out_packets)
{
    inPackets_  = in_packets;
    outPackets_ = out_packets;
}

// TODO: Implement this
/*
void TCPConnection::recvThreadFunc(TCPConnection *connection)
{
    while (!connection->stop_) {
        std::this_thread::sleep_for(100ns);
    }
}

void TCPConnection::sendThreadFunc(TCPConnection *connection)
{
    while (!connection->stop_) {
        std::this_thread::sleep_for(100ns);
    }
}
*/

}; // namespace pirks::networking
