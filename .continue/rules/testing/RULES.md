# Testing Guidelines for Pirks

## Overview
Pirks uses Google Test (gtest) as its primary testing framework. Tests are organized by component and include unit tests, integration tests, and platform-specific tests.

## Test Organization

### Directory Structure
```
test/
├── common-test/              # Common utilities tests
│   └── CircularBufferTest.cpp
├── debug/                    # Debug utility tests  
│   └── memory_utils_test.cpp
└── microphone-test/          # Audio capture integration tests
    └── MicrophoneTest.cpp
```

### Test Categories

1. **Unit Tests** - Test individual components in isolation
2. **Integration Tests** - Test component interactions
3. **Platform-Specific Tests** - Windows/macOS/Linux specific functionality

## Running Tests

### Build with Tests (Default)
```bash
mkdir build && cd build
cmake ..  # BUILD_TESTS=ON by default
cmake --build .
```

### Run All Tests
```bash
# From build directory
ctest --output-on-failure

# Or run test executables directly
./test/common-test/CircularBufferTest
./test/microphone-test/MicrophoneTest
```

### Run Specific Tests
```bash
# Run specific test case
./test/microphone-test/MicrophoneTest --gtest_filter=AudioInput.CaptureDevice

# Run all tests in a group
./test/common-test/CircularBufferTest --gtest_filter=CircularBuffer.*

# List all available tests
./test/common-test/CircularBufferTest --gtest_list_tests
```

### Disable Tests (if needed)
```bash
cmake -DBUILD_TESTS=OFF ..
```

## Writing Unit Tests

### Basic Test Structure
```cpp
#include <gtest/gtest.h>
#include "YourHeader.h"

TEST(ClassName, TestName) {
    // Arrange
    YourClass obj;
    
    // Act
    auto result = obj.doSomething();
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(obj.getValue(), 42);
}
```

### CircularBuffer Testing Example
```cpp
#include <gtest/gtest.h>
#include "CircularBuffer.h"
#include "debug/CircularBufferToStr.h"

TEST(CircularBuffer, Initialization) {
    CircularBuffer<int> buff;
    
    // Verify initial state
    EXPECT_TRUE(buff.isEmpty());
    EXPECT_FALSE(buff.isFull());
    EXPECT_TRUE(buff.isActive());
    EXPECT_EQ(buff.peek(), std::nullopt);
}

TEST(CircularBuffer, BasicPushPop) {
    CircularBuffer<int> buff{7};  // Capacity: 8
    
    buff.push(1);
    buff.push(2);
    
    EXPECT_EQ(buff.pop(), 1);
    EXPECT_EQ(buff.pop(), 2);
    EXPECT_TRUE(buff.isEmpty());
}

TEST(CircularBuffer, OverflowBehavior) {
    CircularBuffer<int> buff{3};  // Capacity: 4
    
    buff.push(1);
    buff.push(2);
    buff.push(3);
    buff.push(4);  // Fill capacity
    
    buff.push(5);  // Should overwrite oldest (1)
    
    EXPECT_EQ(buff.pop(), 2);  // 1 is gone
    EXPECT_EQ(buff.pop(), 3);
    EXPECT_EQ(buff.pop(), 4);
    EXPECT_EQ(buff.pop(), 5);
}

TEST(CircularBuffer, TimeoutBehavior) {
    CircularBuffer<int> buff{3};
    
    // Empty buffer with timeout should return nullopt
    auto result = buff.pop(50ms);
    EXPECT_FALSE(result.has_value());
    
    buff.push(42);
    result = buff.pop(50ms);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}
```

### Testing with Matchers
```cpp
#include <gtest/gtest.h>

TEST(MyClass, Validation) {
    MyClass obj;
    
    // Use Google Test matchers for better readability
    EXPECT_THAT(obj.getValue(), testing::Ge(0));
    EXPECT_THAT(obj.getName(), testing::StartsWith("Test"));
    EXPECT_THAT(obj.getItems(), testing::SizeIs(5));
    
    // Custom matcher
    auto IsPositive = testing::PredicateFormatterIsTrue([](int x) {
        return x > 0;
    }, "is positive");
    
    EXPECT_THAT(calculateResult(), IsPositive);
}
```

## Writing Integration Tests

### Audio Capture Integration Test Example
```cpp
#include <gtest/gtest.h>
#include "AudioInputFactory.h"

#ifdef WINDOWS
#include "ComInitializer.h"
static ::pirks::platform_windows::ComInitializer g_com_initializer;
#endif

TEST(AudioInput, CaptureDevice) {
    using namespace audio::capture_audio;
    
    AudioInputFactory factory;
    const auto names = factory.getAudioSources();
    
    // Verify at least one device available
    ASSERT_GE(names.size(), 1u);
    
    // Create input from first device
    auto device = factory.create(
        names.at(0), 
        2,           // channels (stereo)
        48000,       // sample rate
        288000,      // frame size
        nullptr      // no callback
    );
    
    ASSERT_TRUE(device != nullptr);
}

TEST(AudioInput, GetSamples) {
    using namespace audio::capture_audio;
    
    AudioInputFactory factory;
    const auto names = factory.getAudioSources();
    
    std::vector<float> sample_buffer(1024);
    
    for (const auto& name : names) {
        auto device = factory.create(name, 2, 48000, 96000, nullptr);
        ASSERT_TRUE(device != nullptr);
        
        // Capture multiple samples
        for (int i = 0; i < 8; i++) {
            auto result = device->sample(sample_buffer);
            
            // Should get valid data or NoData (not Error)
            EXPECT_NE(result, CaptureResult::Error);
            
            if (result == CaptureResult::OK) {
                // Verify buffer has reasonable values
                bool hasNonZero = std::any_of(
                    sample_buffer.begin(), 
                    sample_buffer.end(),
                    [](float f) { return std::abs(f) > 0.001f; }
                );
                EXPECT_TRUE(hasNonZero);
            }
        }
    }
}
```

### Networking Integration Test Example
```cpp
#include <gtest/gtest.h>
#include "udp_net/UDPConnection.h"
#include <thread>
#include <chrono>

TEST(UDPIntegration, SendReceive) {
    using namespace pirks::networking;
    
    // Create packet queues
    auto inPackets = std::make_shared<PacketsQueue>(16);
    auto outPackets = std::make_shared<PacketsQueue>(16);
    
    // Create connection on test port
    UDPConnection server;
    EXPECT_NO_THROW(server.create(inPackets, outPackets));
    
    // Give server time to bind
    std::this_thread::sleep_for(100ms);
    
    // TODO: Send test packet from client socket
    // TODO: Verify it appears in inPackets queue
    
    // Cleanup happens automatically via RAII
}
```

## Platform-Specific Tests

### Windows COM Initialization
**CRITICAL**: All Windows tests using COM must initialize it first.

```cpp
#ifdef WINDOWS
#include "ComInitializer.h"

// Must be static and at file/namespace scope
static ::pirks::platform_windows::ComInitializer g_com_initializer;
#endif

TEST(WindowsAudio, WasapiCapture) {
    // Now safe to use COM/WASAPI
    auto devices = getWasapiDevices();
    ASSERT_FALSE(devices.empty());
}
```

### Conditional Test Execution
```cpp
TEST(CrossPlatform, PlatformBehavior) {
#ifdef WINDOWS
    EXPECT_TRUE(isWindowsFeatureAvailable());
#elif defined(MACOS)
    EXPECT_TRUE(isMacosFeatureAvailable());
#elif defined(LINUX)
    EXPECT_TRUE(isLinuxFeatureAvailable());
#endif
}
```

## Test Fixtures

### Using TestFixtures for Setup/Teardown
```cpp
class CircularBufferFixture : public ::testing::Test {
protected:
    void SetUp() override {
        buffer_ = std::make_unique<CircularBuffer<int>>(10);
    }
    
    void TearDown() override {
        buffer_.reset();
    }
    
    std::unique_ptr<CircularBuffer<int>> buffer_;
};

TEST_F(CircularBufferFixture, InitialState) {
    EXPECT_TRUE(buffer_->isEmpty());
}

TEST_F(CircularBufferFixture, PushPopCycle) {
    buffer_->push(1);
    buffer_->push(2);
    EXPECT_EQ(buffer_->pop(), 1);
    EXPECT_EQ(buffer_->pop(), 2);
}
```

### Parameterized Tests
```cpp
class BufferSizeTest : public ::testing::TestWithParam<size_t> {};

TEST_P(BufferSizeTest, CapacityCheck) {
    size_t requested = GetParam();
    CircularBuffer<int> buf(requested);
    
    // Verify actual capacity is N+1
    EXPECT_EQ(buf.bufferCapacity(), requested + 1);
}

INSTANTIATE_TEST_SUITE_P(
    VariousSizes,
    BufferSizeTest,
    ::testing::Values(1, 5, 10, 16, 32, 64, 128)
);
```

## Mocking and Dependency Injection

### Creating Mocks for Dependencies
```cpp
#include <gmock/gmock.h>

class MockAudioInput : public IAudioInput {
public:
    MOCK_METHOD(CaptureResult, sample, (std::vector<float>&), (override));
};

TEST(AudioProcessor, HandlesNoData) {
    MockAudioInput mock_input;
    
    // Expect 3 calls returning NoData, then OK
    EXPECT_CALL(mock_input, sample(::testing::_))
        .WillOnce(::testing::Return(CaptureResult::NoData))
        .WillOnce(::testing::Return(CaptureResult::NoData))
        .WillOnce(::testing::Return(CaptureResult::NoData))
        .WillOnce(::testing::DoAll(
            ::testing::SetArgPointee<0>(std::vector<float>{1.0f, 2.0f}),
            ::testing::Return(CaptureResult::OK)
        ));
    
    AudioProcessor processor(&mock_input);
    auto result = processor.processFrame();
    
    EXPECT_TRUE(result.has_value());
}
```

## Performance Testing

### Benchmarking with Tests
```cpp
#include <chrono>

TEST(CircularBuffer, PushPopPerformance) {
    constexpr size_t ITERATIONS = 100000;
    CircularBuffer<int> buf(1024);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Push phase
    for (size_t i = 0; i < ITERATIONS; i++) {
        buf.push(i);
    }
    
    // Pop phase
    for (size_t i = 0; i < ITERATIONS; i++) {
        buf.pop();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    spdlog::info("Processed {} items in {} microseconds", 
                 ITERATIONS * 2, duration.count());
    
    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 100000);  // < 100ms
}
```

## Debugging Tests

### Enabling Debug Output in Tests
```cpp
TEST(DebugTest, VerboseOutput) {
    // Enable debug logging for test
    spdlog::set_level(spdlog::level::debug);
    
    MyClass obj;
    obj.doSomething();  // Will produce debug logs
    
    // Reset to default level
    spdlog::set_level(spdlog::level::info);
}
```

### Using Test Breakpoints
```cpp
TEST(Debugging, StepThrough) {
    bool breakpoint = false;
    
    setupConditions();
    
    // Set breakpoint here in debugger
    if (breakpoint) {
        processData();
    }
    
    verifyResults();
}
```

## CI/CD Integration

### Running Tests in CI
```yaml
# .github/workflows/test.yml example
name: Run Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: recursive
      
      - name: Configure CMake
        run: cmake -B build -DBUILD_TESTS=ON
      
      - name: Build
        run: cmake --build build
      
      - name: Test
        run: ctest --test-dir build --output-on-failure
```

## Common Testing Pitfalls

### Issue: Tests fail intermittently (flaky tests)
**Solution**: Add proper synchronization and timeouts
```cpp
// ❌ WRONG: Race condition
TEST(FlakyTest, ConcurrentAccess) {
    CircularBuffer<int> buf(10);
    
    std::thread producer([&]() {
        for (int i = 0; i < 100; i++) buf.push(i);
    });
    
    int count = 0;
    while (!buf.isEmpty()) {
        buf.pop();
        count++;
    }
    
    EXPECT_EQ(count, 100);  // May fail due to race!
    
    producer.join();
}

// ✅ CORRECT: Proper synchronization
TEST(ReliableTest, ConcurrentAccess) {
    CircularBuffer<int> buf(10);
    std::mutex mutex;
    std::atomic<bool> done{false};
    
    std::thread producer([&]() {
        for (int i = 0; i < 100; i++) buf.push(i);
        done = true;
    });
    
    int count = 0;
    while (!done || !buf.isEmpty()) {
        auto item = buf.pop(10ms);
        if (item.has_value()) {
            std::lock_guard lock(mutex);
            count++;
        }
    }
    
    producer.join();
    EXPECT_EQ(count, 100);
}
```

### Issue: Platform-specific tests run on wrong platform
**Solution**: Use conditional compilation or SKIP
```cpp
TEST(PlatformSpecific, WindowsOnly) {
#ifndef WINDOWS
    GTEST_SKIP() << "Windows-only test";
#endif
    
    // Windows-specific code
    testWindowsFeature();
}
```

### Issue: Tests depend on external resources (audio devices, network ports)
**Solution**: Check availability and skip gracefully
```cpp
TEST(AudioTest, DeviceCapture) {
    AudioInputFactory factory;
    auto devices = factory.getAudioSources();
    
    if (devices.empty()) {
        GTEST_SKIP() << "No audio devices available";
    }
    
    // Proceed with test
    testDevice(devices[0]);
}

TEST(NetworkTest, PortBinding) {
    int port = findAvailablePort(47990, 48990);
    
    if (port == -1) {
        GTEST_SKIP() << "No available ports in range";
    }
    
    // Use found port
    testWithPort(port);
}
```

## Test Coverage Goals

- **Unit Tests**: Aim for >80% line coverage on core utilities
- **Integration Tests**: Cover all major component interactions
- **Platform Tests**: Ensure each platform has equivalent functionality tests
- **Edge Cases**: Test boundary conditions, error paths, and timeout scenarios

## References

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Mock Cookbook](https://github.com/google/googletest/blob/main/googlemock/docs/cook_book.md)
- [Testing C++ Concurrency](https://www.modernescpp.com/index.php/testing-c-concurrency)
