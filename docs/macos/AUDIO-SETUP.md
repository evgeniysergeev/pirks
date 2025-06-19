# Creating audio input sources in macOS

Die to system limitations on a macOS, it is not possible to get system sounds  
from a desktop. But, you can install third-party software for this:  
- "BlackHole": https://github.com/ExistentialAudio/BlackHole
- "SoundFlower": https://github.com/mattingalls/Soundflower/releases/

For "BlackHole", you can use already built cask from homebrew:
```
brew install --cask blackhole-2ch
```
And use "BlackHole 2ch" audio input source in create() function, for example.

The same for "SoundFlower": 
```
brew install --cask soundflower
```
