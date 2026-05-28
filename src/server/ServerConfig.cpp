#include "ServerConfig.h"

#include <iostream>

void ServerConfig::addOptions(CLI::App &args)
{
    Config::addOptions(args);

    args.add_flag("-t,--tcp", isTCP_, "Use TCP/IP for networking");
    args.add_flag("-u,--udp", isUDP_, "Use UDP for networking");
    args.add_option(
            "--control-plane-address",
            controlPlaneConfig_.bindAddress,
            "Control plane WebSocket TLS bind address");
    args.add_option(
            "--control-plane-port",
            controlPlaneConfig_.port,
            "Control plane WebSocket TLS port");
    args.add_option(
            "--control-plane-cert",
            controlPlaneConfig_.certificateChainFile,
            "Control plane TLS certificate chain file in PEM format");
    args.add_option(
            "--control-plane-key",
            controlPlaneConfig_.privateKeyFile,
            "Control plane TLS private key file in PEM format");
}

bool ServerConfig::parseOptions([[maybe_unused]] CLI::App &args)
{
    Config::parseOptions(args);

    if (isTCP_ && isUDP_) {
        std::cout << "You can not use both TCP and UDP connection types at the same time."
                  << std::endl;
        return false;
    }

    if (controlPlaneConfig_.certificateChainFile.empty()
        != controlPlaneConfig_.privateKeyFile.empty())
    {
        std::cout << "Control plane TLS requires both certificate and private key files."
                  << std::endl;
        return false;
    }

    if (isTCP_) {
        connectionType_ = ConnectionType::TCP;
    }

    if (isUDP_) {
        connectionType_ = ConnectionType::UDP;
    }

    if (connectionType_ == ConnectionType::Default) {
        connectionType_ = ConnectionType::UDP;
    }

    return true;
}
