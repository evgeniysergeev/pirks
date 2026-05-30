# Текущая карта проекта для DevSecOps pipeline

Дата фиксации: 2026-05-30.

Цель документа: зафиксировать текущее устройство проекта Pirks перед внедрением
DevSecOps pipeline, чтобы CI jobs, security checks и future hardening
настраивались от реальной структуры репозитория.

## 1. Команды, использованные для snapshot

```powershell
git status --short --untracked-files=all
git submodule status --recursive
cmake --version
ctest --version
rg "add_(library|executable|subdirectory)|gtest_discover_tests|add_test|option\(" CMakeLists.txt cmake src test -n
ctest -N --test-dir build
ctest -N --test-dir build\test
cmake --build build --target help
```

## 2. Git state

Текущее рабочее дерево содержит один новый TODO-документ:

```text
?? docs/TODO/DEVSECOPS-PIPELINE.md
```

После этого snapshot дополнительно добавлен текущий файл:

```text
docs/DEVSECOPS-PROJECT-MAP.md
```

Submodules зафиксированы так:

```text
a4e5560c5d05bef5dac0c948fb305195c3b8c254 third-party/CLI11 (v1.7.1-533-ga4e5560)
5beeeb81d9c328db99cde3631a81ed6cd92606a5 third-party/TPCircularBuffer (heads/master)
4d6ee56f1bdfd12d1172ed489d7ebc3760e33f9f third-party/clang-format-all (heads/master)
a07c5af66e3a3d41b8cafb65d5826886f36f1537 third-party/deferral (v1.0.0-1-ga07c5af)
79524ddd08a4ec981b7fea76afd08ee05f83755d third-party/spdlog (v1.2.1-2548-g79524ddd)
```

DevSecOps-выводы:

- CI checkout должен использовать recursive submodules.
- Изменения в `third-party` должны требовать review submodule SHA и обновления
  `docs/THIRD-PARTY.md`; само правило зафиксировано в разделе
  `Change policy` этого файла.
- Для SBOM и license audit нужно учитывать и submodules, и зависимости,
  скачиваемые через CMake FetchContent.

## 3. Версии локальных инструментов

```text
cmake version 4.2.3
ctest version 4.2.3
```

DevSecOps-выводы:

- Для Windows CI нужно явно выбрать toolchain: MSVC, MinGW или оба варианта.
- Для статического анализа CI должен генерировать свежий
  `compile_commands.json` через `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.
- Обычный PR pipeline должен переопределять `TEST_MICROPHONE=OFF`.

## 4. Top-level структура

```text
CMakeLists.txt
cmake/
docs/
scripts/
src/
test/
third-party/
```

Назначение директорий:

- `cmake/`: общие CMake-модули, platform definitions, compiler flags,
  dependency setup, test options.
- `src/common/`: общая библиотека, config, string helpers, debug memory utils,
  Windows RAII helpers.
- `src/audio/capture_audio/`: platform-specific аудиозахват:
  Linux PulseAudio, Windows WASAPI, macOS Objective-C/CoreAudio path.
- `src/networking/`: TCP, UDP и control-plane WebSocket/TLS код.
- `src/server/`: executable `pirks-server`.
- `test/`: GoogleTest-based test executables.
- `third-party/`: vendored/submodule dependencies.
- `scripts/`: developer scripts for formatting and cppcheck.
- `docs/`: setup/build docs и текущие TODO-документы.

## 5. CMake options и общие build rules

Top-level options:

```cmake
option(BUILD_TESTS "Build and run tests" ON)
option(TEST_MICROPHONE "Build and test microphone" ON)
```

Общие compiler rules:

- `default_compiler_flags` задает C++23.
- GCC/Clang-like builds используют `-Werror`, `-Wall`, `-Wextra`,
  `-Wpedantic`, `-Wconversion`, `-Wshadow`, `-Wformat=2` и другие строгие флаги.
- MSVC builds используют `/W4` и `/WX`.
- Если `CMAKE_BUILD_TYPE` не задан, проект выставляет `Release`.
- Для `Debug` добавляется `DEBUG=1`, для остальных build types:
  `RELEASE=1` и `NDEBUG`.

Platform definitions:

- macOS: `UNIX`, `MACOS`, `PROJECT_PLATFORM="macOS"`;
- Linux: `UNIX`, `LINUX`, `PROJECT_PLATFORM="Linux"`;
- Windows: `WINDOWS`, `WIN32_LEAN_AND_MEAN`,
  `PROJECT_PLATFORM="Windows"`.

DevSecOps-выводы:

- CI должен проверять как минимум Debug build.
- Release build с hardening flags стоит добавить отдельным job.
- Warning-as-error уже включен, поэтому compiler matrix может выявлять новые
  предупреждения как build failures.

## 6. CMake targets

Основные targets:

```text
common              static library
capture_audio       static library
tcp_net             static library
udp_net             static library
control_plane       static library
pirks-server        executable
```

Test targets:

```text
common-test
common-debug-test
control-plane-test
microphone-test
```

Third-party/build helper targets в текущем build:

```text
CLI11
spdlog
gtest
gtest_main
gmock
gmock_main
```

## 7. Target dependency map

`common`:

- sources: config, debug memory utils, circular buffer, string utils,
  platform Windows helpers;
- links: `default_compiler_flags`;
- exposes include directories for common headers and generated `version.h`.

`capture_audio`:

- common headers: `AudioInputFactory.h`, `CaptureResult.h`, `IAudioInput.h`,
  `IAudioInputFactory.h`;
- Linux: `LinuxAudioInputFactory`, `PulseAudioInput`, `PkgConfig::PULSEAUDIO`;
- Windows: WASAPI/MMDevice/MMCSS helpers, links `avrt`, `mmdevapi`, `uuid`,
  `ksguid`;
- macOS: Objective-C/Objective-C++ capture files, TPCircularBuffer,
  `-fno-objc-arc` for manual retain/release files;
- links: `common`, `default_compiler_flags`.

`tcp_net`:

- sources: `TCPConnection.h`, `TCPConnection.cpp`;
- links: `common`, `default_compiler_flags`, `boost::beast`.

`udp_net`:

- sources: `UDPConnection.h`, `UDPConnection.cpp`;
- links: `common`, `default_compiler_flags`.

`control_plane`:

- sources: control-plane config/message/server;
- requires `find_package(OpenSSL REQUIRED)`;
- links: `common`, `default_compiler_flags`, `Boost::asio`, `boost::beast`,
  `OpenSSL::SSL`, `OpenSSL::Crypto`;
- Windows extra links: `ws2_32`, `mswsock`, `crypt32`, `bcrypt`.

`pirks-server`:

- sources: `main.cpp`, `ServerConfig`, `Server`;
- links: `capture_audio`, `control_plane`, `udp_net`, `tcp_net`,
  `default_compiler_flags`, `${EXTERNAL_LIBRARIES}`.

DevSecOps-выводы:

- SAST и fuzzing в первую очередь стоит направить на `control_plane`,
  `networking` и config parsing.
- Dependency/security jobs должны проверять OpenSSL, Boost/Beast и platform
  audio dependencies.
- macOS job должен учитывать Objective-C files и microphone permission context.

## 8. Test map

CTest обнаружен через `gtest_discover_tests`.

Текущий важный нюанс:

```text
ctest -N --test-dir build       -> Total Tests: 0
ctest -N --test-dir build\test  -> Total Tests: 52
```

Причина: `enable_testing()` вызывается в `test/CMakeLists.txt`, а не в корневом
`CMakeLists.txt`. До изменения CMake CI должен запускать CTest из build test
subdirectory:

```powershell
ctest --test-dir build\ci\test -C Debug --output-on-failure --parallel 4
```

или на Ninja/single-config build:

```bash
ctest --test-dir build/ci/test --output-on-failure --parallel 4
```

Более чистая альтернатива: перенести `enable_testing()` в корневой
`CMakeLists.txt`, чтобы работал стандартный запуск:

```bash
ctest --test-dir build/ci --output-on-failure --parallel 4
```

Текущий discovered test count:

```text
common-test: 17 tests on current Windows/MinGW build
common-debug-test: 25 tests
control-plane-test: 7 tests
microphone-test: 3 tests
total with TEST_MICROPHONE=ON: 52 tests
estimated total with TEST_MICROPHONE=OFF: 49 tests
```

Hardware-sensitive tests:

```text
AudioInput.CaptureDevice
AudioInput.DefaultAudioSourceName
AudioInput.GetSamples
```

DevSecOps-выводы:

- PR CI должен использовать `-DTEST_MICROPHONE=OFF`.
- `microphone-test` нужен manual или self-hosted job с настроенным audio device.
- Root-level CTest behavior нужно исправить или явно учесть в workflow.

## 9. External dependency map

Submodules:

- `third-party/clang-format-all`: helper script for formatting.
- `third-party/spdlog`: logging fallback if packaged spdlog is not found.
- `third-party/CLI11`: command-line parser, always added via subdirectory.
- `third-party/TPCircularBuffer`: macOS microphone capture buffer.
- `third-party/deferral`: included fallback if packaged deferral is not found.

FetchContent/network dependencies:

- GoogleTest: `https://github.com/google/googletest/archive/v1.17.x.zip`.
- Boost Beast v2: `https://github.com/boostorg/beast/archive/refs/heads/develop.zip`.

System/package dependencies:

- OpenSSL is required by `control_plane`.
- Linux audio requires `pkg-config`, `libpulse`, `libpulse-simple`.
- Windows links system libraries for networking and WASAPI.
- macOS uses Objective-C/Objective-C++ runtime and codesign for microphone test
  bundle when available.

DevSecOps-выводы:

- FetchContent URLs should be pinned by immutable version and `URL_HASH`, or
  replaced with controlled dependency provisioning in CI.
- Beast uses the `develop` branch archive, which is risky for reproducibility.
- SBOM tooling must scan both repository contents and generated dependency
  metadata.
- Dependency review should flag changes in `.gitmodules`, submodule SHAs and
  CMake dependency declarations.

## 10. Existing developer quality scripts

Format:

```bash
./scripts/clang-format-all.sh
```

Notes:

- script formats `src` and `test`;
- CI can run it and then check `git diff --exit-code -- src test`.

Cppcheck:

```bash
./scripts/cppcheck-all.sh > cppcheck.log
```

Current cppcheck properties:

- creates `cppcheck-build`;
- configures CMake with `CMAKE_EXPORT_COMPILE_COMMANDS=ON`;
- runs `cppcheck --enable=all`;
- uses `--std=c++23`;
- uses `--check-level=exhaustive`;
- uses `--inconclusive`;
- uses `scripts/suppressions.txt`;
- excludes `_deps`;
- currently does not exclude all `third-party`.

DevSecOps-выводы:

- Existing cppcheck script is better for nightly/full scan.
- PR scan should be faster and probably exclude `third-party`.
- CI should publish `cppcheck.log` as artifact for full scans.

## 11. Documentation map

Relevant docs:

```text
README.md
docs/HOWTO-BUILD.md
docs/SETUP-DEV-ENV.md
docs/THIRD-PARTY.md
docs/COMMIT-TAGS.md
docs/windows/AUDIO-SETUP.md
docs/linux/AUDIO-SETUP.md
docs/macos/AUDIO-SETUP.md
```

Current `docs/THIRD-PARTY.md` lists:

- clang-format-all;
- CLI11;
- spdlog;
- TPCircularBuffer.

Notable gap:

- `deferral`, GoogleTest, Boost/Beast and system dependencies are not fully
  represented in `docs/THIRD-PARTY.md`.
- License identifiers and exact versions/SHAs are not listed.

DevSecOps-выводы:

- License compliance gate should first improve `docs/THIRD-PARTY.md`.
- SBOM can become the machine-readable companion to `docs/THIRD-PARTY.md`.

## 12. Initial CI baseline commands from this map

Windows/MSVC-style baseline:

```powershell
cmake -S . -B build\ci-windows -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build\ci-windows --config Debug --parallel
ctest --test-dir build\ci-windows\test -C Debug --output-on-failure --parallel 4
```

Linux/macOS Ninja baseline:

```bash
cmake -S . -B build/ci -G Ninja -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/ci --parallel
ctest --test-dir build/ci/test --output-on-failure --parallel 4
```

Format baseline:

```bash
./scripts/clang-format-all.sh
git diff --exit-code -- src test
```

Cppcheck nightly baseline:

```bash
./scripts/cppcheck-all.sh > build/reports/cppcheck.log
```

## 13. Immediate TODOs before first required CI gate

1. Decide whether to move `enable_testing()` to root `CMakeLists.txt` or keep
   running `ctest` from `build/*/test`.
2. Add `TEST_MICROPHONE=OFF` to all default PR CI configure commands.
3. Pin FetchContent dependencies by immutable version and checksum.
4. Expand `docs/THIRD-PARTY.md` with dependency versions, SHAs and licenses.
5. Split cppcheck into fast PR profile and exhaustive scheduled profile.
6. Add `scripts/ci/*` wrappers so local and CI commands stay aligned.
7. Add recursive submodule checkout to CI.
8. Decide Windows CI compiler: MSVC for production parity, MinGW for current
   local parity, or both.
