# Common Utilities Component Rules

## Overview
The `src/common/` directory contains shared utilities, data structures, and platform-specific abstractions used throughout the Pirks codebase. These components provide foundational functionality for thread-safe operations, configuration management, and cross-platform compatibility.

## Core Components

### 1. CircularBuffer (`CircularBuffer.h`)

**Purpose**: Thread-safe circular buffer with timeout support for packet queuing.

#### Key Features
- **Thread-Safe**: Can be accessed from multiple threads simultaneously
- **Timeout Support**: Blocking operations with configurable timeouts
- **Overflow Handling**: Oldest elements overwritten when capacity exceeded
- **Capacity + 1**: Actual capacity is `constructor_arg + 1` for implementation reasons

#### Usage Patterns

```cpp
#include "CircularBuffer.h"

// Create buffer with capacity for N+1 elements
CircularBuffer<PacketInfo> packets(16);  // Capacity: 17

// Push single element
packets.push(packet);

// Push multiple elements (array)
int values[] = {1, 2, 3};
packets.push(values);

// Push from vector
std::vector<int> vec = {4, 5, 6};
packets.push(vec);

// Pop single element (blocking with timeout)
auto value = packets.pop(100ms);  // Returns std::optional<T>
if (value.has_value()) {
    process(value.value());
}

// Pop multiple elements into vector
std::vector<int> batch;
auto count = packets.pop(batch, 5, 50ms);  // Get up to 5 items, wait 50ms

// Peek without removing
auto first = packets.peek();  // Non-blocking peek
auto firstWithTimeout = packets.peek(100ms);  // Blocking peek

// Check state
if (packets.isEmpty()) { /* ... */ }
if (packets.isFull()) { /* ... */ }
if (packets.isActive()) { /* ... */ }

// Stop buffer (no more operations allowed)
packets.stop();
```

#### Implementation Details

**Capacity Calculation**: 
- Constructor argument `N` creates buffer with capacity `N + 1`
- This is intentional for power-of-two optimizations in the underlying implementation

**Thread Safety Model**:
- Uses mutex-protected read/write indices
- Condition variables for blocking operations
- Lock-free peek when possible (optimization)

#### Common Pitfalls

```cpp
// ❌ WRONG: Assuming capacity equals constructor argument
CircularBuffer<int> buf(10);  // Actual capacity is 11, not 10
EXPECT_EQ(buf.bufferCapacity(), 11u);  // Correct expectation

// ❌ WRONG: Not checking optional result
auto val = buf.pop();  // May return std::nullopt if empty
int x = val;  // Compilation error! Must check has_value() first

// ✅ CORRECT: Check before using
if (val.has_value()) {
    int x = val.value();
}

// ❌ WRONG: Using stopped buffer
buf.stop();
buf.push(42);  // Undefined behavior!

// ✅ CORRECT: Check active state
if (buf.isActive()) {
    buf.push(42);
}
```

### 2. Configuration System (`config/Config.h`)

**Purpose**: Base class for command-line argument parsing using CLI11.

#### Inheritance Pattern

```cpp
#include "config/Config.h"

class ServerConfig : public pirks::config::Config {
protected:
    void addOptions(CLI::App& args) override;
    bool parseOptions(CLI::App& args) override;

private:
    int port_;
    bool debug_;
};

// Usage in main()
ServerConfig config;
auto ret = config.parseArgs(
    "Pirks Video Streaming Server",  // description
    "pirks-server",                  // app name
    PROJECT_VERSION,                 // version string
    argc, argv
);

if (config.shouldExit()) {
    return ret;  // Help/version requested
}
```

#### Adding New Options

```cpp
void ServerConfig::addOptions(CLI::App& args) {
    // Boolean flag
    args.add_flag("--debug", debug_, "Enable debug logging");
    
    // Value option with default
    args.add_option("--port", port_, "Server port")
        ->default_val(47990)
        ->check(CLI::PositiveNumber);
    
    // Required string option
    std::string requiredValue;
    args.add_option("-r,--required", requiredValue, "Required value")
        ->required();
    
    // Enum option
    static const std::map<std::string, ConnectionType> typeMap = {
        {"udp", ConnectionType::UDP},
        {"tcp", ConnectionType::TCP}
    };
    std::string typeStr;
    args.add_option("--type", typeStr, "Connection type")
        ->default_val("udp")
        ->expected(1)
        ->transform(CLI::CheckedTransformer(typeMap, CLI::ignore_case));
}

bool ServerConfig::parseOptions(CLI::App& args) {
    // Validate cross-option constraints
    if (port_ < 1024 && !isPrivilegedPortAllowed_) {
        args.failure_message() << "Port must be >= 1024 or use --privileged";
        return false;
    }
    return true;
}
```

### 3. Debug Utilities (`debug/`)

#### CircularBufferToStr (`CircularBufferToStr.h`)

**Purpose**: Convert CircularBuffer to string representation for debugging/testing.

```cpp
#include "debug/CircularBufferToStr.h"

CircularBuffer<int> buf(5);
buf.push(1);
buf.push(2);
buf.push(3);

std::string repr = CircularBufferToStr(buf);  // Returns "{1,2,3}"

// Useful in tests
EXPECT_EQ(CircularBufferToStr(buf), "{1,2,3}");
```

#### Memory Utils (`memory_utils.h`)

**Purpose**: Memory inspection and debugging helpers.

```cpp
#include "debug/memory_utils.h"

// Print memory dump
pirks::debug::printMemoryDump(buffer.data(), buffer.size());

// Check for specific patterns
if (pirks::debug::containsPattern(data, pattern)) {
    spdlog::info("Pattern found!");
}
```

### 4. String Utilities (`str_utils.h`)

**Purpose**: Common string manipulation functions.

```cpp
#include "str_utils.h"

// Join strings with separator
std::vector<std::string> parts = {"a", "b", "c"};
auto joined = pirks::join(parts, ",");  // Returns "a,b,c"

// Format helpers
auto formatted = pirks::format("Value: {}", 42);  // Returns "Value: 42"
```

### 5. Exit Codes (`ExitCode.h`)

**Purpose**: Standardized exit codes for the application.

```cpp
#include "ExitCode.h"

using namespace pirks;

int main() {
    try {
        // Application logic
        return ExitCode::OK;
    } catch (const std::exception& e) {
        spdlog::critical("Error: {}", e.what());
        return ExitCode::ExceptionThrown;
    }
}
```

**Defined Codes**:
- `ExitCode::OK` = 0
- `ExitCode::ConfigurationError` = 1
- `ExitCode::ExceptionThrown` = 2
- Add more as needed for specific error categories

### 6. Deferral Utility (`deferral.h`)

**Purpose**: RAII-based deferred execution (similar to Go's `defer`).

```cpp
#include "deferral.h"

void processFile(const std::string& filename) {
    auto file = openFile(filename);
    
    defer {
        spdlog::info("Cleaning up file: {}", filename);
        closeFile(file);
    };
    
    // If exception thrown here, cleanup still runs
    processFileContent(file);
}

// More complex example with capture
void scopedOperation() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    defer [this]() {
        stats_.operationCount++;
        spdlog::debug("Completed operation #{}", stats_.operationCount);
    };
    
    // Operation logic
}
```

## Windows-Specific Utilities (`windows/`)

### WinHandle (`WinHandle.h`)

**Purpose**: RAII wrapper for Windows handles (sockets, events, etc.).

```cpp
#include "windows/WinHandle.h"

void createEvent() {
    auto event = pirks::platform_windows::WinHandle(
        CreateEvent(nullptr, FALSE, FALSE, nullptr)
    );
    
    if (!event) {
        throw std::runtime_error("Failed to create event");
    }
    
    // Use event...
    SetEvent(event.get());
    
    // Automatically closed when 'event' goes out of scope
}

// Access raw handle
HANDLE rawHandle = event.get();
```

### Interface<T> (`Interface.h`)

**Purpose**: Smart pointer for COM interfaces with automatic reference counting.

```cpp
#include "windows/Interface.h"

void useComInterface() {
    pirks::platform_windows::Interface<IAudioClient> audioClient;
    
    // Query interface
    hr = device->Activate(
        __uuidof(IAudioClient),
        IID_PPV_ARGS(audioClient.put())
    );
    
    if (SUCCEEDED(hr) && audioClient) {
        // Use audioClient...
        audioClient->Initialize(...);
        
        // Automatically released when 'audioClient' goes out of scope
    }
}
```

### ComInitializer (`ComInitializer.h`)

**Purpose**: RAII COM library initialization.

**CRITICAL**: Must be initialized before any COM calls on Windows.

```cpp
#include "windows/ComInitializer.h"

// Global or static initializer (must exist before COM usage)
static pirks::platform_windows::ComInitializer g_comInitializer;

void useComObjects() {
    // Safe to use COM here
    CoCreateInstance(...);
}
```

## Platform Detection Macros

Defined in `cmake/platform_definitions.cmake`:

```cpp
#ifdef WINDOWS
    // Windows-specific code
#elif defined(MACOS)
    // macOS-specific code
#elif defined(LINUX)
    // Linux-specific code
#endif

// Platform name string
std::string platformName = PROJECT_PLATFORM;  // "Windows", "macOS", or "Linux"
```

## Design Patterns in Common Utilities

### 1. RAII (Resource Acquisition Is Initialization)
- `WinHandle`: Automatic handle cleanup
- `Interface<T>`: COM reference counting
- `ComInitializer`: COM library lifecycle management
- `defer`: Guaranteed cleanup execution

### 2. Template Metaprogramming
- `CircularBuffer<T>`: Generic container for any type
- Type-safe operations through template constraints

### 3. Strategy Pattern
- `Config` base class with platform-specific implementations
- Swappable configuration strategies

### 4. Builder Pattern (CLI11 integration)
```cpp
args.add_option("--port", port_, "Port number")
    ->default_val(47990)
    ->check(CLI::PositiveNumber)
    ->expected(1);
```

## Testing Guidelines for Common Utilities

### CircularBuffer Tests (`test/common-test/CircularBufferTest.cpp`)

**Key Test Categories**:
1. **Initialization**: Empty state, capacity verification
2. **Basic Operations**: push/pop/peek functionality
3. **Overflow Behavior**: Oldest element replacement when full
4. **Timeout Handling**: Blocking operations with timeouts
5. **Batch Operations**: Multi-element push/pop
6. **State Management**: stop() and isActive()

**Example Test Structure**:
```cpp
TEST(CircularBuffer, BasicUsage) {
    CircularBuffer<int> buf{7};  // Capacity: 8
    
    EXPECT_TRUE(buf.isEmpty());
    
    buf.push(1);
    buf.push(2);
    EXPECT_EQ(buf.pop(), 1);
    EXPECT_EQ(buf.pop(), 2);
    EXPECT_TRUE(buf.isEmpty());
}

TEST(CircularBuffer, TimeoutBehavior) {
    CircularBuffer<int> buf{3};
    
    // Empty buffer with timeout should return nullopt
    auto result = buf.pop(50ms);
    EXPECT_FALSE(result.has_value());
    
    buf.push(42);
    result = buf.pop(50ms);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result, 42);
}
```

### Configuration Tests

```cpp
TEST(ServerConfig, ParsePort) {
    char* argv[] = {(char*)"program", (char*)"--port", (char*)"8080"};
    
    ServerConfig config;
    auto ret = config.parseArgs(desc, name, version, 4, argv);
    
    EXPECT_EQ(ret, ExitCode::OK);
    EXPECT_EQ(config.port(), 8080);
}

TEST(ServerConfig, InvalidPort) {
    char* argv[] = {(char*)"program", (char*)"--port", (char*)"-1"};
    
    ServerConfig config;
    auto ret = config.parseArgs(desc, name, version, 4, argv);
    
    EXPECT_EQ(ret, ExitCode::ConfigurationError);
}
```

## Performance Considerations

### CircularBuffer Optimization

1. **Capacity Planning**: Choose appropriate capacity to minimize overflows
   ```cpp
   // Profile your workload first
   auto avgPacketsPerSecond = measureThroughput();
   auto processingTimeMs = measureProcessingTime();
   
   // Capacity should handle burst scenarios
   size_t capacity = (avgPacketsPerSecond * processingTimeMs / 1000) * 2;
   CircularBuffer<Packet> buffer(capacity);
   ```

2. **Batch Operations**: Prefer batch pop/push for better throughput
   ```cpp
   // Less efficient: multiple single operations
   while (!queue.isEmpty()) {
       auto item = queue.pop();
       process(item);
   }
   
   // More efficient: single batch operation
   std::vector<Item> batch;
   queue.pop(batch, MAX_BATCH_SIZE);
   for (const auto& item : batch) {
       process(item);
   }
   ```

3. **Avoid Unnecessary Copies**: Use move semantics where possible
   ```cpp
   CircularBuffer<std::unique_ptr<Resource>> buffer(10);
   
   // Move instead of copy
   buffer.push(std::make_unique<Resource>(...));
   ```

## Code Style Specifics

- Prefer `std::optional` return types for operations that may fail
- Use `spdlog` consistently across all utilities
- Document thread-safety guarantees in header comments
- Include units in timeout parameters (e.g., `100ms`, not just `100`)
- Use strong exceptions guarantee where possible (RAII helps)

## Common Pitfalls & Solutions

### Issue: CircularBuffer capacity confusion
**Problem**: Constructor argument doesn't match actual capacity
```cpp
// Solution: Remember N+1 rule
CircularBuffer<T> buf(N);  // Capacity is N+1, not N
EXPECT_EQ(buf.bufferCapacity(), static_cast<size_t>(N + 1));
```

### Issue: COM initialization order on Windows
**Problem**: COM calls before CoInitialize
```cpp
// Solution: Use static ComInitializer at file scope
static pirks::platform_windows::ComInitializer g_comInit;

// Then safe to use COM anywhere in this translation unit
```

### Issue: Deferral capture semantics
**Problem**: Capturing references that become invalid
```cpp
// ❌ WRONG: Reference may dangle
std::string temp = getValue();
defer { log(temp); };  // 'temp' destroyed before defer runs!

// ✅ CORRECT: Capture by value or ensure lifetime
std::string temp = getValue();
defer [value = std::move(temp)]() { log(value); };
```

## References

- **CircularBuffer**: Inspired by lock-free queue patterns, adapted for simplicity
- **CLI11 Documentation**: https://github.com/CLIUtils/CLI11
- **spdlog Quick Start**: https://github.com/gabime/spdlog#quick-start
- **RAII Best Practices**: C++ Core Guidelines I.1-I.45
