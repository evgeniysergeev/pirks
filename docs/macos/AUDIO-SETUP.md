# macOS microphone access

`microphone-test` needs macOS microphone permission before it can open an
audio capture device. The test is built as an app bundle so macOS can read
`NSMicrophoneUsageDescription` from its `Info.plist`.

Run the test bundle once and allow microphone access when macOS prompts:

```
./build/test/microphone-test/microphone-test.app/Contents/MacOS/microphone-test --gtest_filter=AudioInput.CaptureDevice
```

If the prompt does not appear, open **System Settings > Privacy & Security >
Microphone** and enable `microphone-test`.

## Running under a debugger

If the test passes when started directly but skips under a debugger, macOS is
most likely denying microphone access to the debug launcher. In **System
Settings > Privacy & Security > Microphone**, enable the application that starts
LLDB, for example Visual Studio Code, CLion, Xcode, Terminal, or iTerm.

Prefer debugging the executable inside the app bundle:

```
./build/test/microphone-test/microphone-test.app/Contents/MacOS/microphone-test
```

Avoid launching a copied standalone executable outside the bundle, because then
macOS cannot use this bundle's `Info.plist` and stable bundle identifier for the
microphone permission decision.

To reset the permission decision for the test bundle:

```
tccutil reset Microphone org.pirks.microphone-test
```

The macOS log line below can appear while CoreAudio scans system plugins and is
not usually the reason the test skips:

```
[plugin] AddInstanceForFactory: No factory registered for id ...
```

CoreMediaIO can also print warnings while it configures audio conversion or
while the process is exiting:

```
CMIO_Unit_Converter_Audio.cpp:... AudioConverterSetProperty(dbca) failed (1886547824)
CMIOHardware.cpp:... CMIOObjectGetPropertyData the System is exiting
CMIO_DALA_System.cpp:... error 1970171760 (unop)
```

These lines come from Apple's media stack on stderr. If the GoogleTest output
still reports `[  PASSED  ]`, the microphone capture path completed
successfully.

## Capturing system audio

Due to system limitations on macOS, it is not possible to get system sounds
from the desktop directly. You can install third-party software for this:

- "BlackHole": https://github.com/ExistentialAudio/BlackHole
- "SoundFlower": https://github.com/mattingalls/Soundflower/releases/

For "BlackHole", you can use already built cask from homebrew:

```
brew install --cask blackhole-2ch
```

And use "BlackHole 2ch" audio input source in the `create()` function, for
example.

The same for "SoundFlower":

```
brew install --cask soundflower
```
