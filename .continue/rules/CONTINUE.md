# Pirks Project Guide

## 1. Project Overview

**Pirks** is a video streaming engine based on open-source projects Sunshine and Moonlight, licensed under AGPL-3.0. It provides real-time audio/video capture and streaming capabilities across multiple platforms (Windows, macOS, Linux).

### Key Technologies
- **Language**: C++23
- **Build System**: CMake 3.15+
- **Testing Framework**: Google Test (gtest)
- **Logging**: spdlog
- **CLI Parsing**: CLI11
- **Platform APIs**: 
  - Windows: WASAPI, COM
  - macOS: Core Audio
  - Linux: ALSA/PulseAudio

### High-Level Architecture
```
┌─────────────────┐
│   Server        │ ← Main application entry point
├─────────────────┤
│ Networking      │ ← TCP/UDP connection handling
├─────────────────┤
│ Audio Capture   │ ← Platform-specific audio input
├─────────────────┤
│ Common          │ ← Shared utilities and data structures
└─────────────────┘
```

## 2. Getting Started

### Prerequisites

**Required Software:**
- CMake 3.15 or higher
- C++23 compatible compiler:
  - GCC 11+ with full C++23 support
  - Clang 14+ 
  - MSVC 2022 (Visual Studio)
- Git with submodule support

**Platform-Specific Requirements:**
- **Windows**: Windows SDK, COM support
- **macOS**: Xcode Command Line Tools
- **Linux**: Development libraries for audio capture (ALSA/PulseAudio)

### Installation Instructions

1. **Clone the repository with submodules:**
```bash
git clone --recurse-submodules https://github.com/evgeniysergeev/pirks.git
cd pirks
```

Or if you already cloned without submodules:
```bash
git submodule update --init --recursive
```

2. **Build the project:**
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

3. **Run tests (enabled by default):**
```bash
# Tests are built automatically with BUILD_TESTS=ON (default)
./test/test_runner  # or equivalent test executable
```

### Basic Usage Examples

**Running the server:**
```bash
# Default UDP mode
./pirks-server

# With debug logging
./pirks-server --debug

# Specify port
./pirks-server --port 47990

# Show help
./pirks-server --help
```

## 3. Project Structure

### Main Directories

```
pirks/
├── .continue/rules/          # Continue AI rules and documentation
├── cmake/                    # CMake configuration modules
│   ├── default_compiler_flags.cmake
│   ├── platform_definitions.cmake
│   ├── build_type.cmake
│   └── ...
├── docs/                     # Documentation files
│   ├── SETUP-DEV-ENV.md      # Development environment setup
│   ├── HOWTO-BUILD.md        # Build instructions
│   └── THIRD-PARTY.md        # Third-party library info
├── scripts/                  # Utility scripts
│   ├── clang-format-all.sh   # Format all source files
│   └── cppcheck-all.sh       # Static analysis
├── src/                      # Source code
│   ├── common/               # Shared utilities and data structures
│   │   ├── CircularBuffer.h  # Thread-safe circular buffer implementation
│   │   ├── config/           # Configuration handling
│   │   ├── debug/            # Debug utilities
│   │   └── windows/          # Windows-specific code
│   ├── audio/                # Audio capture subsystem
│   │   └── capture_audio/    # Platform-specific audio input
│   │       ├── windows/      # WASAPI implementation
│   │       ├── macos/        # Core Audio implementation
│   │       └── linux/        # ALSA/PulseAudio implementation
│   ├── networking/           # Network communication layer
│   │   ├── tcp_net/          # TCP connection (not yet implemented)
│   │   └── udp_net/          # UDP connection
│   └── server/               # Main server application
│       ├── Server.h/cpp      # Server implementation
│       ├── ServerConfig.h/cpp # Configuration parsing
│       └── main.cpp          # Application entry point
├── test/                     # Test suites
│   ├── common-test/          # Common utilities tests
│   ├── debug/                # Debug utility tests
│   └── microphone-test/      # Audio capture tests
├── third-party/              # External dependencies (git submodules)
│   ├── CLI11/                # Command-line argument parsing
│   ├── spdlog/               # Logging library
│   ├── deferral/             # RAII defer utility
│   └── TPCircularBuffer/     # Circular buffer for macOS audio
├── CMakeLists.txt            # Main build configuration
├── .clang-format             # Code formatting rules
└── README.md                 # Project readme
```

### Key Files and Their Roles

**Core Components:**
- `src/server/main.cpp` - Application entry point, initializes server with config
- `src/server/Server.h/cpp` - Main server class managing connections and packet queues
- `src/common/CircularBuffer.h` - Thread-safe circular buffer for packet management
- `src/networking/IConnection.h` - Interface for network connections

**Configuration:**
- `cmake/default_compiler_flags.cmake` - Compiler flags (C++23, warnings)
- `cmake/platform_definitions.cmake` - Platform-specific definitions
- `src/version.h.in` - Template for version header generation

**Platform-Specific Code:**
- Windows: `src/audio/capture_audio/windows/WasapiAudioInput.h/cpp`
- macOS: `src/audio/capture_audio/macos/MacAudioInput.h/cpp`
- Linux: `src/audio/capture_audio/linux/LinuxAudioInputFactory.h/cpp`

## 4. Development Workflow

### Coding Standards

**Style Guide:**
- Based on Google C++ Style Guide and CppCoreGuidelines
- 4-space indentation (no tabs)
- Brackets on same line as control statements
- Empty line at end of files
- Line length: 80 chars preferred, max 120 chars

**Enforcement:**
```bash
# Format all source files
./scripts/clang-format-all.sh

# Run static analysis
./scripts/cppcheck-all.sh > cppcheck.log
```

**IDE Setup (VS Code):**
Add to User Settings for code rulers:
```json
"editor.rulers": [
    {"column": 80, "color": "#ff9900"},
    100,
    {"column": 120, "color": "#9f0af5"}
]
```

### Testing Approach

**Test Framework:** Google Test (gtest)

**Running Tests:**
```bash
# Build with tests enabled (default)
cmake -DBUILD_TESTS=ON ..
cmake --build .

# Run specific test
./test/microphone-test/MicrophoneTest
```

**Test Categories:**
- `test/common-test/` - Unit tests for common utilities (CircularBuffer, etc.)
- `test/debug/` - Debug utility tests
- `test/microphone-test/` - Audio capture integration tests

### Build and Deployment Process

**Build Types:**
- Debug: Full symbols, no optimization
- Release: Optimized, stripped symbols
- RelWithDebInfo: Optimized with debug info

**Platform-Specific Builds:**
```bash
# Windows (MSVC)
cmake -G "Visual Studio 17 2022" ..

# Linux/macOS (Unix Makefiles or Ninja)
cmake -G Ninja ..
cmake --build . -j$(nproc)
```

**Contribution Guidelines:**
1. Follow existing code style (run clang-format before committing)
2. Add tests for new functionality
3. Update documentation in `docs/` as needed
4. Ensure cross-platform compatibility where applicable
5. Use meaningful commit messages following conventional commits

## 5. Key Concepts

### Domain-Specific Terminology

- **Packet Queue**: Thread-safe buffer for network packets using CircularBuffer
- **Audio Capture**: Platform-specific audio input from microphones/devices
- **Connection Type**: Network protocol (UDP default, TCP planned)
- **Frame Size**: Number of audio samples per capture frame
- **Sample Rate**: Audio sampling frequency (e.g., 48000 Hz)

### Core Abstractions

**1. CircularBuffer (`src/common/CircularBuffer.h`)**
```cpp
// Thread-safe circular buffer with timeout support
CircularBuffer<PacketInfo> packets(16);  // Capacity: 17 elements
packets.push(packet);                    // Add packet
auto value = packets.pop();              // Remove and return packet
auto optional = packets.pop(100ms);      // Pop with timeout
```

**2. IConnection Interface (`src/networking/IConnection.h`)**
```cpp
// Abstract interface for network connections
class IConnection {
    virtual void create(std::shared_ptr<PacketsQueue> in, 
                       std::shared_ptr<PacketsQueue> out) = 0;
};
```

**3. Audio Input Factory Pattern**
```cpp
// Platform-specific audio device creation
AudioInputFactory factory;
auto devices = factory.getAudioSources();
auto input = factory.create(deviceName, channels, sampleRate, frameSize);
```

### Design Patterns Used

1. **Factory Pattern**: `AudioInputFactory` for platform-specific audio device creation
2. **Strategy Pattern**: `IConnection` interface with UDP/TCP implementations
3. **RAII**: Resource management throughout (WinHandle, Interface<T>)
4. **Observer Pattern**: Audio notification callbacks for device changes
5. **Template Metaprogramming**: `CircularBuffer<T>` generic container

## 6. Common Tasks

### File Operations by Platform

**Linux/macOS (Bash):**
```bash
# List files recursively
ls -R src/

# Find specific file types
find . -name "*.cpp" -o -name "*.h"

# Create directories
mkdir -p src/audio/capture_audio/newplatform/

# View file contents
cat src/server/main.cpp
head -20 src/common/CircularBuffer.h
```

**Windows (PowerShell):**
```powershell
# List files recursively
Get-ChildItem -Path src -Recurse | Select-Object FullName, Length

# Find specific file types
Get-ChildItem -Recurse -Include *.cpp,*.h

# Create directories
New-Item -ItemType Directory -Force -Path "src\audio\capture_audio\newplatform"

# View file contents
Get-Content src\server\main.cpp
Get-Content src\common\CircularBuffer.h -Head 20

# Search for text in files
Select-String -Pattern "eRender" -Path "src\**\*.h"
```

### Adding a New Platform-Specific Feature

1. Create platform-specific directory under relevant module:
```bash
# Linux/macOS
mkdir -p src/audio/capture_audio/newplatform/

# Windows PowerShell
New-Item -ItemType Directory -Force -Path "src\audio\capture_audio\newplatform"
```

2. Implement platform interface:
```cpp
// src/audio/capture_audio/newplatform/NewPlatformAudioInput.h
class NewPlatformAudioInput : public IAudioInput {
    CaptureResult sample(std::vector<float>& sample_out) override;
};
```

3. Update CMakeLists.txt to include new platform files conditionally

4. Add tests in `test/microphone-test/`

### Adding Command-Line Options

1. Edit `src/server/ServerConfig.h`:
```cpp
enum class ConnectionType {
    Default,
    UDP,
    TCP,
    NEW_OPTION  // Add new option
};
```

2. Implement in `ServerConfig.cpp`:
```cpp
void ServerConfig::addOptions(CLI::App& args) {
    args.add_flag("--new-option", newOption_, "Description");
}
```

### Debugging Audio Capture Issues

1. Enable debug logging:
```bash
# Linux/macOS
./pirks-server --debug

# Windows
.\pirks-server.exe --debug
```

2. List available audio devices:
```cpp
AudioInputFactory factory;
auto devices = factory.getAudioSources();
for (const auto& device : devices) {
    spdlog::info("Device: {}", device);
}
```

3. Test with microphone test suite:
```bash
# Linux/macOS
./test/microphone-test/MicrophoneTest --gtest_filter=AudioInput.*

# Windows
.\test\microphone-test\MicrophoneTest.exe --gtest_filter=AudioInput.*
```

### Building for Different Platforms

**Cross-compilation setup:**
```bash
# Linux to Windows (using MinGW)
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/mingw-toolchain.cmake ..

# macOS universal build
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
```

**Windows-specific commands:**
```powershell
# List available CMake generators
cmake --help

# Build with Visual Studio
cmake -G "Visual Studio 17 2022" -B build
cmake --build build --config Release

# Run executable from build directory
.\build\src\server\pirks-server.exe --debug
```

## 7. Troubleshooting

### Common Issues and Solutions

**Issue: Submodules not initialized**
```bash
# Solution
git submodule update --init --recursive
```

**Issue: C++23 feature not supported**
- **Solution**: Upgrade compiler to GCC 11+, Clang 14+, or MSVC 2022
- Check compiler version: `g++ --version` or `clang++ --version`

**Issue: Audio capture fails on Windows**
- Ensure COM is initialized (check `ComInitializer` usage)
- Verify audio device permissions in Windows settings
- Run as administrator if needed

**Issue: Build errors with spdlog**
```bash
# Solution: Reinitialize submodules
git submodule foreach git pull origin master
rm -rf build && mkdir build && cd build
cmake ..
```

**Issue: Tests fail on CI/CD**
- Ensure test audio devices are available (may need virtual audio driver)
- Set environment variables for headless builds
- Check BUILD_TESTS flag is set correctly

### Debugging Tips

1. **Enable verbose logging:**
```cpp
spdlog::set_level(spdlog::level::trace);
```

2. **Use memory debugging tools:**
```bash
# Run with AddressSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_ASAN=ON ..
```

3. **Profile performance:**
```bash
# Build with profiling
cmake -DCMAKE_CXX_FLAGS="-pg" ..
# Run and analyze gprof output
gprof ./pirks-server gmon.out > profile.txt
```

4. **Network debugging:**
```bash
# Capture network traffic
tcpdump -i any port 47990 -w capture.pcap
# Analyze with Wireshark
wireshark capture.pcap
```

## 8. References

### Documentation

- **Setup Guide**: `docs/SETUP-DEV-ENV.md`
- **Build Instructions**: `docs/HOWTO-BUILD.md`
- **Third-Party Libraries**: `docs/THIRD-PARTY.md`
- **macOS Audio Setup**: `docs/macos/AUDIO-SETUP.md`
- **Commit Tags Guide**: `docs/COMMIT-TAGS.md`

### External Resources

**Base Projects:**
- [Sunshine](https://github.com/LizardByte/Sunshine) - Original Windows streaming server (GPL-3.0)
- [Moonlight](https://github.com/moonlight-stream) - Client applications

**Libraries:**
- [spdlog Documentation](https://github.com/gabime/spdlog#quick-start)
- [CLI11 Guide](https://github.com/CLIUtils/CLI11/blob/master/docs/cli11.md)
- [Google Test Primer](https://google.github.io/googletest/primer.html)

**Standards:**
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines.html)

### License

This project is licensed under AGPL-3.0. See `LICENSE` file for details.

---

**Note**: This guide was auto-generated based on the current codebase structure. Some sections may need verification and updates as the project evolves. Always refer to the actual source code and documentation files for the most accurate information.
