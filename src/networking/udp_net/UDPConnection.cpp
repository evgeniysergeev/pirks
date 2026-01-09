#include "UDPConnection.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace pirks::networking
{

UDPConnection::UDPConnection() : stop_ { false }
{
    // This comment needed for clang-format. Without it, this will be in one line
    spdlog::debug("UDPConnection created");
}

UDPConnection::~UDPConnection()
{
    spdlog::debug("UDPConnection destructor");

    stop_ = true;
}

void UDPConnection::create(
        std::shared_ptr<PacketsQueue> in_packets,
        std::shared_ptr<PacketsQueue> out_packets)
{
    inPackets_  = in_packets;
    outPackets_ = out_packets;
}

// TODO: Implement this
/*
void UDPConnection::recvThreadFunc(TCPConnection *connection)
{
    while (!connection->stop_) {
        std::this_thread::sleep_for(100ns);
    }
}

void UDPConnection::sendThreadFunc(TCPConnection *connection)
{
    while (!connection->stop_) {
        std::this_thread::sleep_for(100ns);
    }
}
*/

}; // namespace pirks::networking
