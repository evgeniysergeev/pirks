#include "Client.h"

#include <iostream>
#include <memory>
#include <utility>

#include "ControlPlaneClient.h"

Client::Client(ClientConfig config) : config_ { std::move(config) }
{
    //
}

Client::~Client() = default;

void Client::run()
{
    controlPlaneClient_ = std::make_unique<ControlPlaneClient>(config_.controlPlaneClientConfig());
    controlPlaneClient_->connect();

    std::cout << "Connected to wss://" << controlPlaneClient_->endpoint() << config_.target()
              << '\n';
    if (!controlPlaneClient_->verifyPeer()) {
        std::cout << "TLS peer verification is disabled\n";
    }

    controlPlaneClient_->sendText(config_.message());
    std::cout << "Sent: " << config_.message() << '\n';

    if (!config_.sendOnly()) {
        std::cout << "Received: " << controlPlaneClient_->readText() << '\n';
    }

    controlPlaneClient_->close();
}
