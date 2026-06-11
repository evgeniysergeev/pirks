#include "ControlPlaneClient.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <chrono>
#include <utility>

namespace
{

namespace beast     = boost::beast;
namespace net       = boost::asio;
namespace ssl       = boost::asio::ssl;
namespace websocket = beast::websocket;

using tcp = net::ip::tcp;

void configureTls(ssl::context &context, const ControlPlaneClientConfig &config)
{
    context.set_options(
            ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3);

    if (!config.verifyPeer) {
        context.set_verify_mode(ssl::verify_none);
        return;
    }

    context.set_verify_mode(ssl::verify_peer);
    if (config.caFile.empty()) {
        context.set_default_verify_paths();
    } else {
        context.load_verify_file(config.caFile);
    }
}

void setServerName(
        websocket::stream<beast::ssl_stream<beast::tcp_stream>> &stream,
        const std::string                                       &serverName)
{
    if (serverName.empty()) {
        return;
    }

    if (SSL_set_tlsext_host_name(stream.next_layer().native_handle(), serverName.c_str()) != 1) {
        beast::error_code error {
            static_cast<int>(::ERR_get_error()),
            net::error::get_ssl_category(),
        };
        throw beast::system_error { error };
    }
}

auto portText(std::uint16_t port) -> std::string
{
    return std::to_string(static_cast<unsigned int>(port));
}

} // namespace

class ControlPlaneClient::Impl final
{
public:
    explicit Impl(ControlPlaneClientConfig config)
            : config_ { std::move(config) }
            , sslContext_ { ssl::context::tls_client }
            , resolver_ { ioContext_ }
            , stream_ { ioContext_, sslContext_ }
    {
        if (config_.serverName.empty()) {
            config_.serverName = config_.address;
        }

        configureTls(sslContext_, config_);
    }

public:
    void connect()
    {
        setServerName(stream_, config_.serverName);
        if (config_.verifyPeer) {
            stream_.next_layer().set_verify_callback(
                    ssl::host_name_verification(config_.serverName));
        }

        const auto timeout = std::chrono::milliseconds { config_.timeoutMs };
        beast::get_lowest_layer(stream_).expires_after(timeout);

        const auto endpoints = resolver_.resolve(config_.address, portText(config_.port));
        beast::get_lowest_layer(stream_).connect(endpoints);
        stream_.next_layer().handshake(ssl::stream_base::client);

        beast::get_lowest_layer(stream_).expires_never();
        stream_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        stream_.handshake(endpoint(), config_.target);
    }

    void sendText(std::string_view message)
    {
        stream_.text(true);
        stream_.write(net::buffer(message));
    }

    auto readText() -> std::string
    {
        beast::flat_buffer buffer;
        stream_.read(buffer);
        return beast::buffers_to_string(buffer.data());
    }

    void close()
    {
        beast::error_code error;
        stream_.close(websocket::close_code::normal, error);
    }

    auto endpoint() const -> std::string
    {
        return config_.address + ":" + portText(config_.port);
    }

    bool verifyPeer() const
    {
        return config_.verifyPeer;
    }

private:
    ControlPlaneClientConfig                                config_;
    net::io_context                                         ioContext_;
    ssl::context                                            sslContext_;
    tcp::resolver                                           resolver_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> stream_;
};

ControlPlaneClient::ControlPlaneClient(ControlPlaneClientConfig config)
        : impl_ { std::make_unique<Impl>(std::move(config)) }
{
    //
}

ControlPlaneClient::~ControlPlaneClient() = default;

void ControlPlaneClient::connect()
{
    impl_->connect();
}

void ControlPlaneClient::sendText(std::string_view message)
{
    impl_->sendText(message);
}

auto ControlPlaneClient::readText() -> std::string
{
    return impl_->readText();
}

void ControlPlaneClient::close()
{
    impl_->close();
}

auto ControlPlaneClient::endpoint() const -> std::string
{
    return impl_->endpoint();
}

bool ControlPlaneClient::verifyPeer() const
{
    return impl_->verifyPeer();
}
