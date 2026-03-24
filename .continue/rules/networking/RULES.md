# Networking Component Rules

## Overview
The networking subsystem handles packet transmission and reception for the streaming server. Currently supports UDP with TCP planned for future implementation.

## Architecture

### Core Interface: IConnection

**Location**: `src/networking/IConnection.h`

```cpp
namespace pirks::networking {
    using PacketsQueue = CircularBuffer<PacketInfo>;

    class IConnection {
    public:
        virtual ~IConnection() = default;

        virtual void create(
            std::shared_ptr<PacketsQueue> in_packets,
            std::shared_ptr<PacketsQueue> out_packets) = 0;
    };
}
```

### Connection Types

| Type | Implementation | Status | Port Default |
|------|---------------|--------|--------------|
| UDP | `UDPConnection` | ✅ Implemented | 47990 |
| TCP | `TCPConnection` | ⏳ Not implemented | - |
| libwebsockets | TBD | 🔮 Planned | - |
| enet | TBD | 🔮 Planned | - |
| QUIC | TBD | 🔮 Planned | - |

### PacketInfo Structure

**Location**: `src/networking/PacketInfo.h`

```cpp
struct PacketInfo {
    std::vector<uint8_t> data;      // Packet payload
    sockaddr_in          address;   // Sender/receiver address
    size_t               length;    // Data length
    
    // Constructors, etc.
};
```

## UDP Implementation Details

### Class: UDPConnection (`src/networking/udp_net/UDPConnection.h`)

**Responsibilities**:
- Bind to specified port (default 47990)
- Receive packets from clients → push to `in_packets` queue
- Pop from `out_packets` queue → send to clients
- Handle socket errors gracefully
- Support multiple concurrent connections

### Typical Usage Pattern

```cpp
#include "udp_net/UDPConnection.h"
#include "CircularBuffer.h"

using namespace pirks::networking;

// Create packet queues
auto inPackets = std::make_shared<PacketsQueue>(16);  // Capacity: 17 packets
auto outPackets = std::make_shared<PacketsQueue>(16);

// Create and initialize connection
UDPConnection connection;
connection.create(inPackets, outPackets);

// Receive loop (typically in separate thread)
while (running) {
    auto packet = inPackets->pop(100ms);  // Wait up to 100ms for packet
    if (packet.has_value()) {
        processIncomingPacket(packet.value());
    }
}

// Send loop
prepareResponse();
outPackets->push(response_packet);
```

### Socket Configuration

**UDP-specific settings**:
- Non-blocking I/O with timeout-based polling
- SO_REUSEADDR enabled for quick restarts
- Buffer sizes tuned for streaming (typically 65KB)
- Connectionless protocol - each packet is independent

## TCP Implementation (Planned)

When implementing `TCPConnection`, follow these guidelines:

### Design Requirements

1. **State Management**: Track connected clients with session IDs
2. **Keep-Alive**: Implement heartbeat mechanism to detect dead connections
3. **Backpressure**: Use CircularBuffer capacity to prevent sender overflow
4. **Graceful Shutdown**: Properly close sockets and notify all threads

### Suggested Structure

```cpp
class TCPConnection : public IConnection {
public:
    void create(std::shared_ptr<PacketsQueue> in, 
                std::shared_ptr<PacketsQueue> out) override;
    
private:
    void acceptLoop();      // Accept new connections
    void receiveLoop();     // Read from connected sockets
    void sendLoop();        // Write to connected sockets
    
    struct ClientSession {
        SOCKET socket;
        sockaddr_in address;
        std::vector<uint8_t> receiveBuffer;
        bool active;
    };
    
    std::map<SOCKET, ClientSession> clients_;
    std::mutex clientsMutex_;
};
```

## Packet Flow Architecture

### Server-Side Flow

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│ Network I/O │────▶│ in_packets   │────▶│ Processor   │
│ (UDP/TCP)   │     │ Queue        │     │             │
└─────────────┘     └──────────────┘     └─────────────┘
                                              │
                                              ▼
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│ Network I/O │◀────│ out_packets  │◀────│ Response    │
│ (UDP/TCP)   │     │ Queue        │     │ Generator   │
└─────────────┘     └──────────────┘     └─────────────┘
```

### Thread Safety Considerations

**CRITICAL**: `PacketsQueue` (CircularBuffer) is thread-safe, but:

1. **Producer-Consumer Pattern**: 
   - Network threads push to `in_packets`, pop from `out_packets`
   - Processing threads pop from `in_packets`, push to `out_packets`

2. **Capacity Limits**: 
   - Default capacity: 16 elements (actual: 17 due to implementation)
   - When full, oldest packets are overwritten (circular behavior)
   - Monitor queue depth for backpressure detection

3. **Timeout Operations**:
   ```cpp
   // Non-blocking with timeout
   auto packet = queue.pop(100ms);  // Returns std::optional
   
   if (packet.has_value()) {
       // Process packet
   } else {
       // Timeout - no packets available
   }
   ```

## Error Handling Patterns

### Socket Errors

```cpp
// Check socket error codes
int err = WSAGetLastError();  // Windows
// or
int err = errno;             // POSIX

switch (err) {
    case WSAEWOULDBLOCK:  // Non-blocking socket would block
        // Continue loop, try again
        break;
    case WSAENOTSOCK:     // Invalid socket
        spdlog::error("Invalid socket descriptor");
        cleanup();
        break;
    default:
        spdlog::warn("Socket error: {}", err);
}
```

### Queue Operations

```cpp
// Safe push with capacity check
if (!packetsQueue->isFull()) {
    packetsQueue->push(packet);
} else {
    spdlog::debug("Packet queue full, dropping packet");
    // Optionally implement backpressure or packet prioritization
}

// Safe pop with timeout
auto result = packetsQueue->pop(50ms);
if (!result.has_value()) {
    // No data available within timeout
    continue;  // or yield thread
}
```

## Performance Optimization

### UDP-Specific Optimizations

1. **Buffer Sizes**: Tune socket receive/send buffers
   ```cpp
   int bufSize = 65536;  // 64KB
   setsockopt(socket, SOL_SOCKET, SO_RCVBUF, 
              reinterpret_cast<char*>(&bufSize), sizeof(bufSize));
   ```

2. **Batch Processing**: Process multiple packets per iteration
   ```cpp
   std::vector<PacketInfo> batch;
   auto count = inPackets->pop(batch, 10);  // Get up to 10 packets
   for (const auto& packet : batch) {
       process(packet);
   }
   ```

3. **Zero-Copy Where Possible**: Minimize data copying between buffers

### Monitoring Metrics

Track these metrics for performance tuning:
- Packet queue depth over time
- Packets dropped due to full queues
- Average latency (packet creation → processing)
- Network I/O wait times

## Testing Guidelines

### Unit Tests

Test individual components in isolation:
```cpp
TEST(UDPConnection, BindToPort) {
    auto inQ = std::make_shared<PacketsQueue>(8);
    auto outQ = std::make_shared<PacketsQueue>(8);
    
    UDPConnection conn;
    EXPECT_NO_THROW(conn.create(inQ, outQ));
}

TEST(PacketInfo, Serialization) {
    PacketInfo packet;
    packet.data = {0x12, 0x34, 0x56};
    // Test serialization/deserialization round-trip
}
```

### Integration Tests

Test end-to-end packet flow:
```cpp
TEST(NetworkingIntegration, SendReceive) {
    // Start server on port
    // Connect client socket
    // Send test packet
    // Verify packet appears in in_packets queue
    // Push response to out_packets queue
    // Verify client receives response
}
```

## Platform-Specific Considerations

### Windows
- Initialize Winsock: `WSAStartup(MAKEWORD(2, 2), &wsaData)`
- Use `SOCKET` type instead of `int`
- Close with `closesocket()` not `close()`
- Error codes from `WSAGetLastError()`

### POSIX (Linux/macOS)
- Socket file descriptors are `int`
- Use `close()` to close sockets
- Error codes in `errno`
- May need `<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`

## Security Considerations

1. **Input Validation**: Always validate packet data before processing
2. **DDoS Protection**: Implement rate limiting per IP address
3. **Port Binding**: Bind to specific interfaces when possible (not 0.0.0.0)
4. **Encryption**: Plan for DTLS/SRTP in future implementations

## Code Style Specifics

- Use `spdlog` for all network-related logging
- Prefer RAII wrappers for socket management
- Document expected packet formats and sizes
- Handle all error cases explicitly (no bare `catch (...)`)
- Use meaningful timeout values (not magic numbers)

## Future Enhancements

### Planned Features
- [ ] TCP connection support with session management
- [ ] Packet encryption/decryption layer
- [ ] Quality of Service (QoS) packet prioritization
- [ ] Connection pooling for high-concurrency scenarios
- [ ] Adaptive bitrate based on network conditions

### Research Areas
- QUIC protocol implementation for reduced latency
- WebRTC integration for browser compatibility
- P2P mesh networking for distributed streaming

## References

- [UDP Protocol RFC 768](https://datatracker.ietf.org/doc/html/rfc768)
- [TCP Protocol RFC 793](https://datatracker.ietf.org/doc/html/rfc793)
- Windows Sockets API: `winsock2.h` documentation
- POSIX Socket Programming: "UNIX Network Programming" by W. Richard Stevens
