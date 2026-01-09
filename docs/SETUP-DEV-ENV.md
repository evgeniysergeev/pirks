# Code Style

## clang-format

Code uses this (.clang-format)[../.clang-format] style.
It is based on (Google style guide)[https://google.github.io/styleguide/cppguide.html] and (CppCoreGuidelines)[https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines.html]
    - Uses 4 spaces as indentation.
    - Brackets is on the same line.
    - Empty line at end of file.

To update source code with the selected code style, periodically run 
this command:
```bash
./scripts/clang-format-all.sh
```

## cppcheck

Cppcheck script is available in [cppcheck-all.sh](../scripts/cppcheck-all.sh)
Better to call this file with redirect stdout to file. This way you will see only errors and warnings on the screen
```bash
./scripts/cppcheck-all.sh > cppcheck.log
```

# ./scripts/cppcheck-all.sh > cppcheck.log

## Rullers in Editor

For VS Code, it is good to enable rullers in User settings:
Press Ctrl+Shift+P (for Windows) or Shift+Command+P (for macOS)
to "Open User Settings (JSON)". And add this lines:
```json
    "editor.rulers": [
        {
            "column": 80,    // spacing of 1st column from left
            "color": "#ff9900"
        },
            100,  // 2nd ruler with no color option
        {
            "column": 120,
            "color": "#9f0af5"
        },
    ],
```
