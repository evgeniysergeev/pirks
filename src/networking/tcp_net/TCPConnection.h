#pragma once

#include <atomic>
#include <thread>

#include "../IConnection.h"

namespace pirks::networking
{

class TCPConnection final : public IConnection
{
public:
    TCPConnection();
    ~TCPConnection() override;

private:
    static void recvThreadFunc(TCPConnection *connection);
    static void sendThreadFunc(TCPConnection *connection);

private:
    std::atomic_bool stop_;
    std::thread      recvThread_;
    std::thread      sendThread_;
};

}; // namespace pirks::networking
