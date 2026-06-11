#include "ClientConfig.h"

#include <iostream>

#include "ExitCode.h"

void ClientConfig::addOptions(CLI::App &args)
{
    Config::addOptions(args);

    args.add_option(
                "--control-plane-address",
                controlPlaneAddress_,
                "Control plane WebSocket TLS address")
            ->capture_default_str();
    args.add_option("--control-plane-port", controlPlanePort_, "Control plane WebSocket TLS port")
            ->capture_default_str();
    args.add_option("--target", target_, "WebSocket target path")->capture_default_str();
    args.add_option(
            "--server-name",
            serverName_,
            "TLS SNI and certificate hostname. Defaults to --control-plane-address");
    args.add_option("-m,--message", message_, "Text WebSocket message to send as one frame")
            ->capture_default_str();
    args.add_option("--ca-file", caFile_, "PEM CA file for --verify-peer");
    args.add_option("--timeout-ms", timeoutMs_, "Connect/read/write timeout in ms")
            ->check(CLI::PositiveNumber)
            ->capture_default_str();
    args.add_flag("--verify-peer", verifyPeer_, "Verify the TLS certificate");
    args.add_flag("--send-only", sendOnly_, "Do not wait for a response after sending");
}

bool ClientConfig::parseOptions([[maybe_unused]] CLI::App &args)
{
    Config::parseOptions(args);

    if (controlPlaneAddress_.empty()) {
        std::cout << "--control-plane-address must not be empty" << std::endl;
        return false;
    }

    if (target_.empty() || target_.front() != '/') {
        std::cout << "--target must start with '/'" << std::endl;
        return false;
    }

    if (serverName_.empty()) {
        serverName_ = controlPlaneAddress_;
    }

    return true;
}

auto ClientConfig::controlPlaneClientConfig() const -> ControlPlaneClientConfig
{
    ControlPlaneClientConfig config;
    config.address    = controlPlaneAddress_;
    config.port       = controlPlanePort_;
    config.target     = target_;
    config.serverName = serverName_;
    config.caFile     = caFile_;
    config.timeoutMs  = timeoutMs_;
    config.verifyPeer = verifyPeer_;

    return config;
}
