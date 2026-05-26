# Linux audio capture

The Linux capture backend uses PulseAudio's client API. It also works on
PipeWire systems when the PulseAudio compatibility service is running.

## Build dependencies

Install the PulseAudio development package before configuring CMake.

Debian or Ubuntu:

```
sudo apt-get update
sudo apt-get install -y cmake ninja-build build-essential pkg-config libpulse-dev
```

Fedora:

```
sudo dnf install pulseaudio-libs-devel
```

Arch Linux:

```
sudo pacman -S libpulse
```

## Runtime service

Use either PulseAudio or PipeWire with `pipewire-pulse` enabled. If no
compatible audio server is running, `microphone-test` will skip because no
accessible capture device can be opened.

## Sources

Call `AudioInputFactory::getDefaultAudioSourceName()` to get the current
default source name, then pass that name to `AudioInputFactory::create()`.
`AudioInputFactory::getAudioSources()` returns all explicit PulseAudio sources
reported by the server.

Regular microphone sources usually look like:

```
alsa_input.pci-0000_00_1f.3.analog-stereo
```

System audio monitor sources usually look like:

```
alsa_output.pci-0000_00_1f.3.analog-stereo.monitor
```

Use a microphone source to capture an input device, or a `.monitor` source to
capture the audio currently being played through the matching output sink.

## Tests

From a Linux shell:

```
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --target microphone-test
./build/test/microphone-test/microphone-test --gtest_filter=AudioInput.*
```

From Windows PowerShell using WSL:

```
wsl -d Ubuntu -- bash -lc "cmake -S /mnt/c/Prj/pirks -B ~/pirks-build -DBUILD_TESTS=ON -G Ninja"
wsl -d Ubuntu -- bash -lc "cmake --build ~/pirks-build --target microphone-test"
wsl -d Ubuntu -- bash -lc "~/pirks-build/test/microphone-test/microphone-test --gtest_filter=AudioInput.*"
```

Keep the WSL build directory under the Linux filesystem, for example
`~/pirks-build`, instead of under `/mnt/c`. This avoids CMake permission errors
on Windows-mounted paths.
