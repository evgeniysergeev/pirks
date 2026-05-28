#include "ControlPlaneServer.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <spdlog/spdlog.h>

#include <boost/asio/dispatch.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{

namespace beast     = boost::beast;
namespace http      = beast::http;
namespace net       = boost::asio;
namespace ssl       = boost::asio::ssl;
namespace websocket = beast::websocket;

using tcp = net::ip::tcp;

using EvpPkeyContextPtr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using EvpPkeyPtr        = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using X509Ptr           = std::unique_ptr<X509, decltype(&X509_free)>;

auto openSslErrorMessage(std::string_view operation) -> std::string
{
    char       message[256] {};
    const auto error = ERR_get_error();
    if (error != 0) {
        ERR_error_string_n(error, message, sizeof(message));
    } else {
        std::snprintf(message, sizeof(message), "unknown OpenSSL error");
    }

    return std::string(operation) + ": " + message;
}

void throwOpenSslError(std::string_view operation)
{
    throw std::runtime_error(openSslErrorMessage(operation));
}

void checkOpenSslResult(int result, std::string_view operation)
{
    if (result != 1) {
        throwOpenSslError(operation);
    }
}

auto generatePrivateKey() -> EvpPkeyPtr
{
    EvpPkeyContextPtr keyContext {
        EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr),
        EVP_PKEY_CTX_free,
    };
    if (!keyContext) {
        throwOpenSslError("EVP_PKEY_CTX_new_id");
    }

    checkOpenSslResult(EVP_PKEY_keygen_init(keyContext.get()), "EVP_PKEY_keygen_init");
    checkOpenSslResult(
            EVP_PKEY_CTX_set_rsa_keygen_bits(keyContext.get(), 2048),
            "EVP_PKEY_CTX_set_rsa_keygen_bits");

    EVP_PKEY *rawPrivateKey = nullptr;
    checkOpenSslResult(EVP_PKEY_keygen(keyContext.get(), &rawPrivateKey), "EVP_PKEY_keygen");

    return EvpPkeyPtr { rawPrivateKey, EVP_PKEY_free };
}

void addCertificateNameEntry(X509_NAME *name, const char *field, const char *value)
{
    checkOpenSslResult(
            X509_NAME_add_entry_by_txt(
                    name,
                    field,
                    MBSTRING_ASC,
                    reinterpret_cast<const unsigned char *>(value),
                    -1,
                    -1,
                    0),
            "X509_NAME_add_entry_by_txt");
}

auto generateCertificate(EVP_PKEY *privateKey) -> X509Ptr
{
    X509Ptr certificate { X509_new(), X509_free };
    if (!certificate) {
        throwOpenSslError("X509_new");
    }

    checkOpenSslResult(X509_set_version(certificate.get(), 2), "X509_set_version");
    checkOpenSslResult(
            ASN1_INTEGER_set(X509_get_serialNumber(certificate.get()), 1),
            "ASN1_INTEGER_set");
    if (X509_gmtime_adj(X509_get_notBefore(certificate.get()), 0) == nullptr) {
        throwOpenSslError("X509_gmtime_adj notBefore");
    }
    if (X509_gmtime_adj(X509_get_notAfter(certificate.get()), 60 * 60 * 24) == nullptr) {
        throwOpenSslError("X509_gmtime_adj notAfter");
    }
    checkOpenSslResult(X509_set_pubkey(certificate.get(), privateKey), "X509_set_pubkey");

    X509_NAME *name = X509_get_subject_name(certificate.get());
    if (name == nullptr) {
        throwOpenSslError("X509_get_subject_name");
    }

    addCertificateNameEntry(name, "C", "XX");
    addCertificateNameEntry(name, "O", "Pirks");
    addCertificateNameEntry(name, "CN", "pirks-control-plane");

    checkOpenSslResult(X509_set_issuer_name(certificate.get(), name), "X509_set_issuer_name");
    if (X509_sign(certificate.get(), privateKey, EVP_sha256()) == 0) {
        throwOpenSslError("X509_sign");
    }

    return certificate;
}

auto remoteEndpoint(tcp::socket &socket) -> std::string
{
    beast::error_code error;
    const auto        endpoint = socket.remote_endpoint(error);
    if (error) {
        return "<unknown>";
    }

    return endpoint.address().to_string() + ":"
           + std::to_string(static_cast<unsigned int>(endpoint.port()));
}

void logError(beast::error_code error, std::string_view operation)
{
    if (error) {
        spdlog::warn("Control plane {} failed: {}", operation, error.message());
    }
}

class ControlPlaneSession final: public std::enable_shared_from_this<ControlPlaneSession>
{
public:
    ControlPlaneSession(tcp::socket socket, ssl::context &context, std::string remoteEndpoint)
            : stream_ { std::move(socket), context }
            , remoteEndpoint_ { std::move(remoteEndpoint) }
    {
        //
    }

public:
    void run()
    {
        net::dispatch(
                stream_.get_executor(),
                beast::bind_front_handler(&ControlPlaneSession::onRun, shared_from_this()));
    }

private:
    void onRun()
    {
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
        stream_.next_layer().async_handshake(
                ssl::stream_base::server,
                beast::bind_front_handler(&ControlPlaneSession::onHandshake, shared_from_this()));
    }

    void onHandshake(beast::error_code error)
    {
        if (error) {
            logError(error, "TLS handshake");
            return;
        }

        beast::get_lowest_layer(stream_).expires_never();
        stream_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
        stream_.set_option(
                websocket::stream_base::decorator([](websocket::response_type &response) {
            response.set(
                    http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) + " pirks-control-plane");
        }));

        stream_.async_accept(
                beast::bind_front_handler(&ControlPlaneSession::onAccept, shared_from_this()));
    }

    void onAccept(beast::error_code error)
    {
        if (error) {
            logError(error, "WebSocket accept");
            return;
        }

        spdlog::info("Control plane client connected from {}", remoteEndpoint_);
        doRead();
    }

    void doRead()
    {
        stream_.async_read(
                buffer_,
                beast::bind_front_handler(&ControlPlaneSession::onRead, shared_from_this()));
    }

    void onRead(beast::error_code error, std::size_t bytesTransferred)
    {
        if (error == websocket::error::closed || error == ssl::error::stream_truncated
            || error == net::error::eof)
        {
            spdlog::info("Control plane client disconnected from {}", remoteEndpoint_);
            return;
        }

        if (error) {
            logError(error, "read");
            return;
        }

        spdlog::debug(
                "Control plane received {} {} bytes from {}",
                stream_.got_text() ? "text" : "binary",
                bytesTransferred,
                remoteEndpoint_);
        buffer_.consume(buffer_.size());
        doRead();
    }

private:
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> stream_;
    beast::flat_buffer                                      buffer_;
    std::string                                             remoteEndpoint_;
};

} // namespace

ControlPlaneServer::ControlPlaneServer(ControlPlaneConfig config)
        : config_ { std::move(config) }
        , ioContext_ { 1 }
        , sslContext_ { ssl::context::tls_server }
        , acceptor_ { ioContext_ }
{
    //
}

ControlPlaneServer::~ControlPlaneServer()
{
    stop();
}

void ControlPlaneServer::start()
{
    if (started_) {
        return;
    }

    configureTls();

    const auto address  = net::ip::make_address(config_.bindAddress);
    const auto endpoint = tcp::endpoint { address, config_.port };

    beast::error_code error;
    acceptor_.open(endpoint.protocol(), error);
    if (error) {
        throw std::runtime_error("Control plane open failed: " + error.message());
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), error);
    if (error) {
        throw std::runtime_error("Control plane set_option failed: " + error.message());
    }

    acceptor_.bind(endpoint, error);
    if (error) {
        throw std::runtime_error("Control plane bind failed: " + error.message());
    }

    acceptor_.listen(net::socket_base::max_listen_connections, error);
    if (error) {
        throw std::runtime_error("Control plane listen failed: " + error.message());
    }

    doAccept();
    ioThread_ = std::thread([this] { ioContext_.run(); });
    started_  = true;

    spdlog::info(
            "Control plane is waiting for WSS clients on wss://{}:{}",
            config_.bindAddress,
            config_.port);
}

void ControlPlaneServer::stop()
{
    if (!started_) {
        return;
    }

    net::post(ioContext_, [this] {
        beast::error_code error;
        acceptor_.close(error);
        logError(error, "acceptor close");
    });
    ioContext_.stop();

    if (ioThread_.joinable()) {
        ioThread_.join();
    }

    started_ = false;
}

void ControlPlaneServer::wait()
{
    if (ioThread_.joinable()) {
        ioThread_.join();
    }

    started_ = false;
}

void ControlPlaneServer::configureTls()
{
    sslContext_.set_options(
            ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3
            | ssl::context::single_dh_use);

    if (config_.certificateChainFile.empty() && config_.privateKeyFile.empty()) {
        configureEphemeralCertificate();
        return;
    }

    if (config_.certificateChainFile.empty() || config_.privateKeyFile.empty()) {
        throw std::invalid_argument(
                "Control plane TLS requires both certificate and private key files");
    }

    configureTlsFromFiles();
}

void ControlPlaneServer::configureTlsFromFiles()
{
    sslContext_.use_certificate_chain_file(config_.certificateChainFile);
    sslContext_.use_private_key_file(config_.privateKeyFile, ssl::context::file_format::pem);
}

void ControlPlaneServer::configureEphemeralCertificate()
{
    spdlog::warn(
            "Control plane TLS certificate/key files are not configured; using an ephemeral "
            "self-signed certificate for this process");

    auto  privateKey  = generatePrivateKey();
    auto  certificate = generateCertificate(privateKey.get());
    auto *handle      = sslContext_.native_handle();

    checkOpenSslResult(
            SSL_CTX_use_certificate(handle, certificate.get()),
            "SSL_CTX_use_certificate");
    checkOpenSslResult(SSL_CTX_use_PrivateKey(handle, privateKey.get()), "SSL_CTX_use_PrivateKey");
    checkOpenSslResult(SSL_CTX_check_private_key(handle), "SSL_CTX_check_private_key");
}

void ControlPlaneServer::doAccept()
{
    acceptor_.async_accept(beast::bind_front_handler(&ControlPlaneServer::onAccept, this));
}

void ControlPlaneServer::onAccept(beast::error_code error, tcp::socket socket)
{
    if (error == net::error::operation_aborted) {
        return;
    }

    if (error) {
        logError(error, "accept");
    } else {
        const auto endpoint = remoteEndpoint(socket);
        std::make_shared<ControlPlaneSession>(std::move(socket), sslContext_, endpoint)->run();
    }

    doAccept();
}
