#pragma once

#include <cstdint>
#include <string>

#include "Config.h"
#include "ControlPlaneClient.h"

class ClientConfig final: public Config
{
public:
    auto controlPlaneAddress() const -> const std::string &
    {
        return controlPlaneAddress_;
    }

    auto controlPlanePort() const -> std::uint16_t
    {
        return controlPlanePort_;
    }

    auto target() const -> const std::string &
    {
        return target_;
    }

    auto serverName() const -> const std::string &
    {
        return serverName_;
    }

    auto caFile() const -> const std::string &
    {
        return caFile_;
    }

    auto message() const -> const std::string &
    {
        return message_;
    }

    auto timeoutMs() const -> std::int64_t
    {
        return timeoutMs_;
    }

    bool verifyPeer() const
    {
        return verifyPeer_;
    }

    bool sendOnly() const
    {
        return sendOnly_;
    }

    auto controlPlaneClientConfig() const -> ControlPlaneClientConfig;

protected:
    void addOptions(CLI::App &args) override;
    bool parseOptions(CLI::App &args) override;

private:
    std::string   controlPlaneAddress_ { "127.0.0.1" };
    std::uint16_t controlPlanePort_ { 5102 };
    std::string   target_ { "/" };
    std::string   serverName_;
    std::string   caFile_;
    std::string   message_ { R"({"type":"ping","id":"1","payload":{"source":"pirks-client"}})" };
    std::int64_t  timeoutMs_ { 5000 };
    bool          verifyPeer_ { false };
    bool          sendOnly_ { false };
};
