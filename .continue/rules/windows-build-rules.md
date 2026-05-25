# Windows Build Rules for Pirks Project

## ⚠️ Critical PowerShell Syntax Rules

### ❌ NEVER use && operator (invalid in PowerShell)
`powershell
# WRONG - will FAIL!
cd build; cmake .. && cmake --build .
`

### ✅ USE semicolons or separate lines
`powershell
# CORRECT - using semicolons
cd build; cmake ..; cmake --build . -j8

# Or step-by-step (RECOMMENDED)
cd build
cmake ..
cmake --build . -j8
`

---

## Prerequisites Verification

### Check Required Tools
`powershell
# CMake version (need 3.15+)
cmake --version

# Compiler versions (MinGW GCC 11+ or Clang 14+)
g++ --version
gcc --version

# Available build tools
where ninja        # Ninja build tool
mingw32-make -v    # MinGW make utility
`

### Expected Output
`
cmake version 3.28.x
g++ (MinGW) 15.2.0
C:\msys64\mingw64\bin\ninja.exe
`

---

## Repository Management

### Clone with Submodules

**Option 1: One-command clone**
`powershell
git clone --recurse-submodules https://github.com/evgeniysergeev/pirks.git
cd pirks
`

**Option 2: Separate initialization**
`powershell
git clone https://github.com/evgeniysergeev/pirks.git
cd pirks
git submodule update --init --recursive
`

### Verify Submodules are Loaded
`powershell
git submodule status
# Should show commit hashes, not '-' or empty lines
`

---

## Build Process

### Step 1: Clean Previous Build (if needed)

**⚠️ WARNING**: May fail if executables are running!

`powershell
# Remove old build directory with error suppression
Remove-Item -Recurse -Force "build" -ErrorAction SilentlyContinue

# Create new build directory
New-Item -ItemType Directory -Force -Path "build" | Out-Null
`

**If removal fails:**
1. Check for running processes: Get-Process pirks*
2. Kill any running executables: Stop-Process -Name pirks-server -Force
3. Retry deletion

### Step 2: Configure CMake

`powershell
cd build
cmake ..
`

**Expected output should show:**
`
-- Building for: Ninja
-- The CXX compiler identification is GNU 15.2.0
-- Setting build type to 'Release' as none was specified.
-- Bundled version of spdlog will be used.
-- Configuring done (X.Xs)
-- Build files have been written to: ...
`

**For Visual Studio generator (if installed):**
`powershell
cmake -G "Visual Studio 17 2022" -A x64 ..
`

### Step 3: Build the Project

`powershell
cmake --build . -j8  # Uses 8 parallel jobs (adjust based on CPU)
`

**Or using make directly:**
`powershell
mingw32-make -j8
`

---

## Running Executables

### Pirks Server

**Navigate to executable location:**
`powershell
cd build\src\server
`

**Run with different options:**
`powershell
# Default (UDP, port 47990)
.\pirks-server.exe

# Debug mode
.\pirks-server.exe --debug

# Show help
.\pirks-server.exe --help

# Custom port
.\pirks-server.exe --port 8080
`

### Testing Tools

**Microphone test:**
`powershell
cd build\test\microphone-test
.\microphone-test.exe
`

**Filter specific test:**
`powershell
.\microphone-test.exe --gtest_filter="AudioInput.CaptureDevice"
`

**List all tests:**
`powershell
.\microphone-test.exe --gtest_list_tests
`

---

## Testing with CTest

`powershell
cd build
ctest --output-on-failure -V  # Verbose mode
`

### Build Debug Configuration

`powershell
# Reconfigure for debug builds
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Rebuild everything
cmake --build . --config Debug
`

---

## Troubleshooting

### Issue: "Cannot find any instance of Visual Studio"

**Solution**: Use MinGW/GCC instead (recommended):
`powershell
cmake ..  # Auto-detects MinGW if available
`

### Issue: File access errors when removing build directory

**Cause**: Executables are still running

**Solutions:**
1. Close pirks-server.exe processes manually
2. Kill via Task Manager or PowerShell:
   `powershell
   Stop-Process -Name pirks* -Force
   `
3. Retry deletion command

### Issue: "Generator could not be found"

**Solution**: List available generators and choose one:
`powershell
cmake --help  # Shows all available generators
`

### Issue: Microphone detection fails on Windows

**Critical**: COM initialization must happen BEFORE any WASAPI calls!

**Checklist:**
1. Run as Administrator (sometimes required for audio devices)
2. Verify ComInitializer.h is included before WASAPI usage:
   `cpp
   static ::pirks::platform_windows::ComInitializer g_com_initializer;
   `
3. Check Windows Privacy settings: Settings → Privacy & Security → Microphone

### Issue: C++23 features not supported

**Solution**: Upgrade compiler version:
- GCC: 11+ required
- Clang: 14+ required  
- MSVC: Visual Studio 2022 (17.x) or newer

---

## Diagnostic Commands

### Check for Running Executables
`powershell
Get-Process pirks* -ErrorAction SilentlyContinue
`

### Find All Executables in Build
`powershell
Get-ChildItem -Path build -Recurse -Filter "*pirks*.exe" | Select-Object FullName, Length
`

### View CMake Configuration
`powershell
Get-Content "build\CMakeCache.txt" | Select-String "BUILD_TYPE|CMAKE_GENERATOR|CXX_COMPILER"
`

---

## Quick Reference Card

| Task | Command |
|------|---------|
| **Clone repo** | git clone --recurse-submodules <url> |
| **Init submodules** | git submodule update --init --recursive |
| **Clean build dir** | Remove-Item -Recurse -Force "build" -ErrorAction SilentlyContinue |
| **Create build dir** | mkdir build; cd build |
| **Configure CMake** | cmake .. |
| **Build project** | cmake --build . -j8 |
| **Run server** | cd src\server; .\pirks-server.exe |
| **Debug mode** | .\pirks-server.exe --debug |
| **Run tests** | cd test\microphone-test; .\microphone-test.exe |

---

## Important Notes

1. **Always use forward slashes / in CMake paths** (even on Windows)
2. **Use semicolons ; to separate commands**, not &&
3. **Kill processes before deleting build directory** if it fails
4. **COM initialization is critical for audio capture on Windows** - always declare ComInitializer as static global variable
5. **Run diagnostic tools first** if microphone detection fails

---

## Verification Checklist Before Starting

- [ ] CMake 3.15+ installed
- [ ] GCC/G++ 11+ or Clang 14+ installed
- [ ] Ninja or make available
- [ ] Git submodules initialized
- [ ] No running pirks-server.exe processes
- [ ] PowerShell execution policy allows script execution (if using scripts)
