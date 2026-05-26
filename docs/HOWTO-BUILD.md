# Prepeare

## Clone sources

```
git clone --recurse-submodules https://github.com/evgeniysergeev/pirks.git
```

or

```
git clone https://github.com/evgeniysergeev/pirks.git
git submodule update --init --recursive
```

# Build

For examle you can use this commands

```
mkdir build
cd build
cmake ..
cmake --build .
```

Or install CMake tools in VSCode and use it

## Build in WSL

Install build dependencies inside the WSL distribution:

```
sudo apt-get update
sudo apt-get install -y cmake ninja-build build-essential pkg-config libpulse-dev
```

When building sources stored under `/mnt/c`, prefer a persistent build
directory inside the Linux filesystem. CMake can fail with `Operation not
permitted` when the build directory is on the Windows-mounted filesystem.

From Windows PowerShell:

```
wsl -d Ubuntu -- bash -lc "cmake -S /mnt/c/Prj/pirks -B ~/pirks-build -DBUILD_TESTS=ON -G Ninja"
wsl -d Ubuntu -- bash -lc "cmake --build ~/pirks-build"
```

Run tests directly from the WSL build directory:

```
wsl -d Ubuntu -- bash -lc "~/pirks-build/test/microphone-test/microphone-test --gtest_filter=AudioInput.*"
wsl -d Ubuntu -- bash -lc "~/pirks-build/test/common-test/common-test"
wsl -d Ubuntu -- bash -lc "~/pirks-build/test/common-debug-test/common-debug-test"
```

# Tests

Microphone tests need OS-level microphone permission. On Windows, see
[Windows microphone access](windows/AUDIO-SETUP.md) if `microphone-test.exe`
reports `HRESULT = 0x80070005` or skips with no accessible audio capture
device.

On Linux, see [Linux audio capture](linux/AUDIO-SETUP.md) if CMake cannot find
PulseAudio development files or `microphone-test` skips with no accessible
audio capture device.

On macOS, see [macOS microphone access](macos/AUDIO-SETUP.md) if
`microphone-test` skips with no audio capture devices or no accessible audio
capture device.
