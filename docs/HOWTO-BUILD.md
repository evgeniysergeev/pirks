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