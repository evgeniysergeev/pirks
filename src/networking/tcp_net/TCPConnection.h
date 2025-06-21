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
    std::atomic_bool m_stop;
    std::thread      m_recvThread;
    std::thread      m_sendThread;
};

}; // namespace pirks::networking
