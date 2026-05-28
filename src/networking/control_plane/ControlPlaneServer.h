#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <thread>

#include "ControlPlaneConfig.h"

class ControlPlaneServer final
{
public:
    explicit ControlPlaneServer(ControlPlaneConfig config);
    ~ControlPlaneServer();

public:
    void start();
    void stop();
    void wait();

private:
    void configureTls();
    void configureTlsFromFiles();
    void configureEphemeralCertificate();
    void doAccept();
    void onAccept(boost::system::error_code error, boost::asio::ip::tcp::socket socket);

private:
    ControlPlaneConfig             config_;
    boost::asio::io_context        ioContext_;
    boost::asio::ssl::context      sslContext_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::thread                    ioThread_;
    bool                           started_ { false };
};
