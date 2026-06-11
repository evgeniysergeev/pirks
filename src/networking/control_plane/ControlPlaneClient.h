#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

struct ControlPlaneClientConfig final
{
    std::string   address { "127.0.0.1" };
    std::uint16_t port { 5102 };
    std::string   target { "/" };
    std::string   serverName;
    std::string   caFile;
    std::int64_t  timeoutMs { 5000 };
    bool          verifyPeer { false };
};

class ControlPlaneClient final
{
public:
    explicit ControlPlaneClient(ControlPlaneClientConfig config);
    ~ControlPlaneClient();

public:
    void connect();
    void sendText(std::string_view message);
    auto readText() -> std::string;
    void close();

    auto endpoint() const -> std::string;
    bool verifyPeer() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
