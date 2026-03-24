# Server Component Rules

## Overview
The server component (src/server/) is the main application entry point that orchestrates networking, audio capture, and packet processing for the Pirks video streaming engine.

## Architecture

### Main Components

`
┌─────────────────────────────┐
│         main.cpp            │  Entry point, config parsing
├─────────────────────────────┤
│       Server.h/cpp          │  Core server logic
├─────────────────────────────┤
│    ServerConfig.h/cpp       │  Command-line configuration
└─────────────────────────────┘
         │
         ├──► Networking (UDP/TCP)
         ├──► Audio Capture
         └──► Packet Processing
`

### Class: Server (Server.h)

**Responsibilities**:
- Initialize and manage network connections
- Coordinate packet queues between components
- Handle server lifecycle (start/stop)
- Manage connection type selection

#### Key Methods

`cpp
class Server {
public:
    explicit Server(config::ServerConfig::ConnectionType connectionType);
    ~Server();
    
    void run();   // Start server and begin processing
    void stop();  // Gracefully shutdown
    
private:
    config::ServerConfig::ConnectionType      connectionType_;
    std::unique_ptr<networking::IConnection>  connection_;
    std::shared_ptr<networking::PacketsQueue> inPackets_;
    std::shared_ptr<networking::PacketsQueue> outPackets_;
};
`

#### Typical Server Lifecycle

`cpp
int main(int argc, char** argv) {
    // 1. Parse configuration
    ServerConfig config;
    auto ret = config.parseArgs(description, name, version, argc, argv);
    
    if (config.shouldExit()) {
        return ret;  // Help/version requested
    }
    
    // 2. Set logging level
    if (config.isDebug()) {
        spdlog::set_level(spdlog::level::debug);
    }
    
    // 3. Create and run server
    Server server(config.connectionType());
    server.run();
    
    return ExitCode::OK;
}
`

### Class: ServerConfig (ServerConfig.h)

**Purpose**: Parse command-line arguments using CLI11 framework.

#### Connection Types

`cpp
enum class ConnectionType {
    Default = 0,  ///< Defaults to UDP
    UDP,          ///< Use UDP protocol (implemented)
    TCP,          ///< Use TCP protocol (not yet implemented)
    // TODO: libwebsockets
    // TODO: enet
    // TODO: quic
};
`

#### Adding New Configuration Options

**Step 1**: Add member variable to ServerConfig.h
`cpp
class ServerConfig : public Config {
private:
    int bufferSize_ = 65536;  // Default buffer size
    bool enableEncryption_ = false;
};
`

**Step 2**: Add CLI option in ServerConfig.cpp
`cpp
void ServerConfig::addOptions(CLI::App& args) {
    // Existing options...
    
    // New options:
    args.add_option("--buffer-size", bufferSize_, 
                   "Packet buffer size (bytes)")
        ->default_val(65536)
        ->check(CLI::PositiveNumber);
        
    args.add_flag("--encrypt", enableEncryption_,
                 "Enable packet encryption");
}

bool ServerConfig::parseOptions(CLI::App& args) {
    // Validate cross-option constraints
    if (bufferSize_ > 131072) {  // Max 128KB
        args.failure_message() << "Buffer size cannot exceed 128KB";
        return false;
    }
    
    if (enableEncryption_) {
        spdlog::warn("Encryption is experimental");
    }
    
    return true;
}
`

**Step 3**: Add getter method
`cpp
auto bufferSize() const -> int {
    return bufferSize_;
}

auto isEncryptionEnabled() const -> bool {
    return enableEncryption_;
}
`

## Server Implementation Details

### Packet Queue Management

The server uses two packet queues for bidirectional communication:

`cpp
// In Server constructor or initialization
inPackets_ = std::make_shared<PacketsQueue>(16);  // Incoming from network
outPackets_ = std::make_shared<PacketsQueue>(16); // Outgoing to network

connection_->create(inPackets_, outPackets_);
`

**Flow**:
- Network thread: receives packets → pushes to inPackets_
- Processing thread: pops from inPackets_, processes, pushes to outPackets_
- Network thread: pops from outPackets_, sends to clients

### Connection Type Handling

`cpp
switch (config.connectionType()) {
    case ServerConfig::ConnectionType::Default:
        [[fallthrough]];
    case ServerConfig::ConnectionType::UDP:
        spdlog::info("Using UDP connection on port {}", config.port());
        // Create UDPConnection
        break;
        
    case ServerConfig::ConnectionType::TCP:
        spdlog::critical("TCP not yet implemented");
        return ExitCode::ConfigurationError;
        
    default:
        spdlog::error("Unknown connection type");
        return ExitCode::ConfigurationError;
}
`

### Graceful Shutdown

**Implementation Pattern**:
`cpp
void Server::stop() {
    spdlog::info("Stopping server...");
    
    // Signal threads to stop
    running_ = false;
    
    // Stop packet queues
    if (inPackets_) inPackets_->stop();
    if (outPackets_) outPackets_->stop();
    
    // Wait for threads to finish
    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }
    if (sendThread_.joinable()) {
        sendThread_.join();
    }
    
    // Connection cleanup happens via RAII (unique_ptr destructor)
    spdlog::info("Server stopped");
}
`

## Error Handling

### Exception Safety

The main entry point should catch all exceptions:

`cpp
int main() {
    try {
        ServerConfig config;
        auto ret = config.parseArgs(...);
        
        if (config.shouldExit()) {
            return ret;
        }
        
        Server server(config.connectionType());
        server.run();
        
    } catch (const std::exception& e) {
        spdlog::critical("Exception: {}", e.what());
        return ExitCode::ExceptionThrown;
    } catch (...) {
        spdlog::critical("Unknown exception");
        return ExitCode::ExceptionThrown;
    }
    
    return ExitCode::OK;
}
`

### Configuration Validation

Validate configuration before starting server:

`cpp
bool ServerConfig::parseOptions(CLI::App& args) {
    // Port validation
    if (port_ < 1024 && !allowPrivilegedPorts_) {
        args.failure_message() << "Port must be >= 1024 or use --privileged";
        return false;
    }
    
    // Connection type availability
    if (connectionType_ == ConnectionType::TCP) {
        args.failure_message() << "TCP not yet implemented, use UDP";
        return false;
    }
    
    return true;
}
`

## Logging and Debugging

### Log Levels

`cpp
// In main.cpp or early initialization
if (config.isDebug()) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Debug logging enabled");
} else {
    spdlog::set_level(spdlog::level::info);
}

// For troubleshooting, can enable trace:
spdlog::set_level(spdlog::level::trace);
`

### Key Log Points

`cpp
// Server startup
spdlog::info("{} v{} (Platform: {})", 
             PROJECT_NAME, PROJECT_FULL_VERSION, PROJECT_PLATFORM);

// Connection type
spdlog::info("Connection type: UDP, port: {}", config.port());

// Shutdown
defer {
    spdlog::info("{} v{} exited", PROJECT_NAME, PROJECT_VERSION);
};
`

## Performance Considerations

### Thread Architecture

**Recommended**: Separate threads for I/O and processing

`cpp
void Server::run() {
    // Receive thread: network → inPackets_
    receiveThread_ = std::thread([this]() {
        while (running_) {
            auto packet = inPackets_->pop(100ms);
            if (packet.has_value()) {
                processPacket(packet.value());
            }
        }
    });
    
    // Send thread: outPackets_ → network
    sendThread_ = std::thread([this]() {
        while (running_) {
            auto packet = outPackets_->pop(100ms);
            if (packet.has_value()) {
                sendPacket(packet.value());
            }
        }
    });
    
    // Wait for threads
    receiveThread_.join();
    sendThread_.join();
}
`

### Buffer Sizing

Tune packet queue capacity based on workload:

`cpp
// Default: 16 elements (actual capacity: 17)
auto packets = std::make_shared<PacketsQueue>(16);

// High-throughput scenario: increase capacity
auto highCapacityPackets = std::make_shared<PacketsQueue>(64);

// Low-latency scenario: smaller capacity, faster processing
auto lowLatencyPackets = std::make_shared<PacketsQueue>(8);
`

## Platform-Specific Considerations

### Windows COM Initialization

**CRITICAL**: Server on Windows requires COM initialization:

`cpp
#ifdef WINDOWS
#include "ComInitializer.h"

// Global initializer (file scope)
static ::pirks::platform_windows::ComInitializer g_com_initializer;
#endif

int main() {
    // Now safe to use COM-dependent features
}
`

### Port Binding

Different platforms may have different port restrictions:

`cpp
#ifdef WINDOWS
// Windows: ports < 1024 require administrator privileges
if (port_ < 1024) {
    spdlog::warn("Port {} requires administrator privileges", port_);
}
#endif
`

## Testing the Server

### Unit Tests for Configuration

`cpp
TEST(ServerConfig, ParseDefaultOptions) {
    char* argv[] = {(char*)"pirks-server"};
    
    ServerConfig config;
    auto ret = config.parseArgs("desc", "name", "1.0", 1, argv);
    
    EXPECT_EQ(ret, ExitCode::OK);
    EXPECT_FALSE(config.shouldExit());
    EXPECT_EQ(config.connectionType(), ConnectionType::Default);
}

TEST(ServerConfig, ParseCustomPort) {
    char* argv[] = {(char*)"pirks-server", (char*)"--port", (char*)"8080"};
    
    ServerConfig config;
    auto ret = config.parseArgs("desc", "name", "1.0", 4, argv);
    
    EXPECT_EQ(ret, ExitCode::OK);
    EXPECT_EQ(config.port(), 8080);
}

TEST(ServerConfig, ShowHelp) {
    char* argv[] = {(char*)"pirks-server", (char*)"--help"};
    
    ServerConfig config;
    auto ret = config.parseArgs("desc", "name", "1.0", 3, argv);
    
    EXPECT_TRUE(config.shouldExit());
}
`

### Integration Tests

`cpp
TEST(ServerIntegration, StartStop) {
    ServerConfig config;
    // Configure for test port
    
    Server server(config.connectionType());
    
    EXPECT_NO_THROW(server.run());
    EXPECT_NO_THROW(server.stop());
}
`

## Common Command-Line Usage

`ash
# Default (UDP on port 47990)
./pirks-server

# Custom port
./pirks-server --port 50000

# Debug mode
./pirks-server --debug

# Show help
./pirks-server --help

# Show version
./pirks-server --version
`

## Future Enhancements

### Planned Features

1. **TCP Support**: Implement TCPConnection class
2. **Encryption**: Add DTLS/SRTP support
3. **Multiple Clients**: Handle concurrent connections
4. **Configuration Files**: Support config file in addition to CLI
5. **Web Interface**: Optional web-based management UI

### Research Areas

- Adaptive bitrate based on network conditions
- Quality of Service (QoS) packet prioritization
- Connection pooling for high concurrency
- QUIC protocol implementation

## References

- Main entry point: src/server/main.cpp
- Server class: src/server/Server.h/cpp
- Configuration: src/server/ServerConfig.h/cpp
- Exit codes: src/common/ExitCode.h
- CLI11 documentation: https://github.com/CLIUtils/CLI11

