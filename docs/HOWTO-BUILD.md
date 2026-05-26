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

# Tests

Microphone tests need OS-level microphone permission. On Windows, see
[Windows microphone access](windows/AUDIO-SETUP.md) if `microphone-test.exe`
reports `HRESULT = 0x80070005` or skips with no accessible audio capture
device.

On macOS, see [macOS microphone access](macos/AUDIO-SETUP.md) if
`microphone-test` skips with no audio capture devices or no accessible audio
capture device.
