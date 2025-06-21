#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "../IConnection.h"

namespace pirks::networking
{

// TODO: Implement TCP connection
class TCPConnection final: public IConnection
{
public:
    TCPConnection();
    ~TCPConnection() override;

public:
    void
    create( //
            std::shared_ptr<PacketsQueue> in_packets,
            std::shared_ptr<PacketsQueue> out_packets) override;

private:
    static void recvThreadFunc(TCPConnection *connection);
    static void sendThreadFunc(TCPConnection *connection);

private:
    std::atomic_bool            stop_;
    std::weak_ptr<PacketsQueue> inPackets_;
    std::weak_ptr<PacketsQueue> outPackets_;
    std::thread                 recvThread_;
    std::thread                 sendThread_;
};

}; // namespace pirks::networking
