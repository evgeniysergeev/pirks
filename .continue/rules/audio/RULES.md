# Audio Capture Component Rules

## Overview
The audio capture subsystem handles platform-specific microphone and audio device input. It uses a factory pattern to abstract away platform differences.

## Architecture

### Core Interfaces

**IAudioInput (`src/audio/capture_audio/IAudioInput.h`)**
```cpp
class IAudioInput {
public:
    virtual ~IAudioInput() = default;
    virtual CaptureResult sample(std::vector<float>& sample_out) = 0;
};
```

**IAudioInputFactory (`src/audio/capture_audio/IAudioInputFactory.h`)**
```cpp
class IAudioInputFactory {
public:
    virtual std::vector<std::string> getAudioSources() = 0;
    virtual std::unique_ptr<IAudioInput> create(
        const std::string& device_name,
        uint8_t channels,
        uint32_t sample_rate,
        uint32_t frame_size,
        std::function<void()> endpt_changed_cb) = 0;
};
```

### Platform Implementations

| Platform | Factory Class | Input Class | Technology |
|----------|--------------|-------------|------------|
| Windows | `WinAudioInputFactory` | `WasapiAudioInput` | WASAPI + COM |
| macOS | `MacAudioInputFactory` | `MacAudioInput` | Core Audio |
| Linux | `LinuxAudioInputFactory` | TBD | ALSA/PulseAudio |

## Key Concepts

### CaptureResult Enum
```cpp
enum class CaptureResult {
    OK = 0,           // Successful capture
    NoData = 1,       // No data available yet
    Error = 2         // Capture error occurred
};
```

### Audio Parameters
- **Channels**: Number of audio channels (1=mono, 2=stereo)
- **Sample Rate**: Hz sampling frequency (typically 48000)
- **Frame Size**: Number of samples per capture frame (e.g., 96000 = 2 seconds at 48kHz stereo)

## Windows Implementation Details

### COM Initialization
**CRITICAL**: All Windows audio code requires COM initialization:
```cpp
#include "ComInitializer.h"
static ::pirks::platform_windows::ComInitializer g_com_initializer;
```

This must be declared as a static variable before any WASAPI calls.

### WASAPI Setup Flow
1. Create `DeviceEnumerator` to list devices
2. Select default or specific audio endpoint
3. Activate `IAudioClient` with share mode and stream format
4. Get `IAudioCaptureClient` for data retrieval
5. Start audio client
6. Call `sample()` repeatedly to get audio data

### Audio Notification
Windows supports device change notifications:
```cpp
AudioNotificationImpl notification;
notification.registerCallback([]() {
    // Handle device change
});
```

## macOS Implementation Details

Uses Core Audio framework with `TPCircularBuffer` for thread-safe buffer management.

### Key Classes
- `CaptureDevice`: Wraps AudioDeviceID and handles property listeners
- `MacAudioInput`: Implements IAudioInput using AudioUnit or IOAudioEngine

## Common Patterns

### Sample Collection Pattern
```cpp
std::vector<float> sample_buffer(frame_size);
auto input = factory.create(device, 2, 48000, 96000, nullptr);

while (running) {
    auto result = input->sample(sample_buffer);
    if (result == CaptureResult::OK) {
        // Process sample_buffer data
    } else if (result == CaptureResult::Error) {
        spdlog::error("Audio capture error");
        break;
    }
    // NoData: continue loop, will try again
}
```

### Device Enumeration Pattern
```cpp
AudioInputFactory factory;
auto devices = factory.getAudioSources();

for (const auto& device : devices) {
    spdlog::info("Available device: {}", device);
}

// Create input from first available device
if (!devices.empty()) {
    auto input = factory.create(devices[0], 2, 48000, 96000, nullptr);
}
```

## Testing Guidelines

### Unit Tests
- Test `AudioInputFactory::getAudioSources()` returns at least one device
- Verify sample data is non-zero (silence detection)
- Check buffer sizes match requested frame_size

### Integration Tests (`test/microphone-test/`)
```cpp
TEST(AudioInput, CaptureDevice) {
    AudioInputFactory factory;
    const auto names = factory.getAudioSources();
    ASSERT_GE(names.size(), 1u);
    
    auto device = factory.create(names.at(0), 2, 48000, 288000, nullptr);
    ASSERT_TRUE(device != nullptr);
}

TEST(AudioInput, GetSamples) {
    // Capture multiple samples and verify data integrity
    std::vector<float> sample_in(1024);
    auto device = factory.create(...);
    
    for (int i = 0; i < 8; i++) {
        auto result = device->sample(sample_in);
        ASSERT_TRUE(result == CaptureResult::OK);
    }
}
```

## Platform-Specific Headers

### Windows
- `AudioClientPtr.h` - Smart pointer for IAudioClient
- `DeviceEnumeratorPtr.h` - Smart pointer for IMMDeviceEnumerator
- `AudioUUIDs.h` - CLSID and IID constants
- `AudioFormats.h` - WAVEFORMATEX structures
- `AudioNotificationImpl.h` - Device change notification handler

### macOS
- `CaptureDevice.h` - Audio device wrapper with property listeners
- Uses TPCircularBuffer from third-party for thread-safe buffering

## Common Issues & Solutions

### Issue: No audio devices found on Windows
**Solution**: 
- Check if COM is initialized (`ComInitializer`)
- Verify application has microphone permissions in Windows Settings
- Try running as administrator

### Issue: Audio capture returns NoData repeatedly
**Solution**:
- Increase frame_size parameter
- Check if audio device is actually the default input device
- Verify sample_rate matches device capabilities (try 44100 or 48000)

### Issue: COM initialization errors on Windows
**Solution**:
```cpp
// Ensure this is in your translation unit
static ::pirks::platform_windows::ComInitializer g_com_initializer;
```

### Issue: Memory leaks with audio devices
**Solution**:
- Always use smart pointers (`AudioClientPtr`, `DeviceEnumeratorPtr`)
- Ensure proper cleanup in destructors
- Use RAII pattern for all COM interfaces

## Performance Considerations

1. **Buffer Size**: Larger frame_size = fewer callbacks but more latency
2. **Sample Rate**: 48000 Hz is standard for streaming, 44100 for audio CDs
3. **Thread Safety**: IAudioInput::sample() may be called from multiple threads
4. **Zero-Copy**: Where possible, avoid copying audio data between buffers

## Code Style Specifics

- Always check `CaptureResult` before processing samples
- Use `spdlog` for all audio-related logging
- Prefer `std::vector<float>` for sample buffers (normalized -1.0 to 1.0)
- Document expected parameter ranges in function comments

## References

- [WASAPI Documentation](https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [Core Audio Overview](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/)
- Third-party: `TPCircularBuffer` for macOS implementation
