# Build Troubleshooting Guide

This guide helps resolve common build and runtime issues with Windows Driver Monitor.

## Table of Contents
1. [Pre-Build Checks](#pre-build-checks)
2. [CMake Errors](#cmake-errors)
3. [Compiler Errors](#compiler-errors)
4. [Linker Errors](#linker-errors)
5. [Runtime Errors](#runtime-errors)
6. [Performance Issues](#performance-issues)

---

## Pre-Build Checks

### Verify Prerequisites

**Windows Version:**
```powershell
# PowerShell
winver
# Should be Windows 10 (1903+) or Windows 11
```

**Visual Studio:**
```cmd
# Command Prompt
where cl
# Should show path to cl.exe (C:\Program Files\Microsoft Visual Studio\...)
```

**CMake:**
```cmd
cmake --version
# Should be 3.15 or higher
```

**Git:**
```cmd
git --version
# Should be 2.0 or higher
```

### Verify ImGui Installation

```powershell
# PowerShell - Run from driver-monitor directory
Test-Path "libs/imgui/imgui.h"
Test-Path "libs/imgui/backends/imgui_impl_dx11.cpp"
# Both should return True
```

---

## CMake Errors

### Error: "CMake 3.15 or higher is required"

**Cause:** CMake version too old

**Solution:**
```bash
# Update CMake
# Download from: https://cmake.org/download/
# Or use installer:
choco install cmake  # If using Chocolatey
```

### Error: "Could not find DirectX 11"

**Cause:** Windows SDK not installed

**Solution:**
1. Open Visual Studio Installer
2. Click "Modify"
3. Under "Individual Components", check:
   - Windows 10 SDK (10.0.19041.0 or later)
   - Windows 11 SDK (if on Windows 11)
4. Click "Modify" to install

### Error: "Cannot find imgui.h"

**Cause:** ImGui not downloaded or in wrong location

**Solution:**
```bash
# Verify location
ls libs/imgui/imgui.h

# If missing, download
git clone https://github.com/ocornut/imgui.git libs/imgui

# Verify backends
ls libs/imgui/backends/imgui_impl_dx11.cpp
```

### Error: "Generator not found"

**Cause:** Visual Studio not detected by CMake

**Solution:**
```bash
# Specify generator explicitly
cmake -G "Visual Studio 16 2019" ..
# Or for VS 2022:
cmake -G "Visual Studio 17 2022" ..
```

### Error: "Architecture not specified"

**Cause:** Need to specify x64 architecture

**Solution:**
```bash
cmake -G "Visual Studio 16 2019" -A x64 ..
```

---

## Compiler Errors

### Error: "C2039: 'function' is not a member of 'std'"

**Cause:** C++ standard not set to C++17

**Solution:**
Add to CMakeLists.txt (should already be there):
```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

Then rebuild:
```bash
cmake --build . --clean-first
```

### Error: "Cannot open include file: 'imgui.h'"

**Cause:** ImGui include paths not set correctly

**Solution:**
Verify CMakeLists.txt has:
```cmake
target_include_directories(DriverMonitor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
```

### Error: "Redefinition of 'class ImGuiContext'"

**Cause:** ImGui headers included multiple times

**Solution:**
Ensure headers have include guards and only include imgui.h once per file.

### Error: "'localtime_s': identifier not found"

**Cause:** Missing Windows headers

**Solution:**
Add at top of file:
```cpp
#include <Windows.h>
#include <time.h>
```

### Error: "Unresolved external symbol WinMain"

**Cause:** Entry point not found or wrong subsystem

**Solution:**
Verify main.cpp has:
```cpp
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    // ...
}
```

And CMakeLists.txt has:
```cmake
set_target_properties(DriverMonitor PROPERTIES
    WIN32_EXECUTABLE TRUE
)
```

---

## Linker Errors

### Error: "LNK2019: unresolved external symbol D3D11CreateDeviceAndSwapChain"

**Cause:** DirectX 11 library not linked

**Solution:**
Verify CMakeLists.txt has:
```cmake
target_link_libraries(DriverMonitor PRIVATE
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
)
```

### Error: "LNK2019: unresolved external symbol ImGui::..."

**Cause:** ImGui source files not compiled

**Solution:**
Verify CMakeLists.txt includes:
```cmake
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)
```

And these are in the executable:
```cmake
add_executable(DriverMonitor WIN32 ${ALL_SOURCES})
```

### Error: "LNK2019: unresolved external symbol WinVerifyTrust"

**Cause:** wintrust.lib not linked

**Solution:**
Add to CMakeLists.txt (should already be there):
```cmake
target_link_libraries(DriverMonitor PRIVATE
    wintrust.lib
    crypt32.lib
)
```

### Error: "LNK2019: unresolved external symbol CoCreateInstance"

**Cause:** COM libraries not linked

**Solution:**
Add to CMakeLists.txt:
```cmake
target_link_libraries(DriverMonitor PRIVATE
    wbemuuid.lib
    ole32.lib
    oleaut32.lib
)
```

### Error: "LNK1104: cannot open file 'd3d11.lib'"

**Cause:** Windows SDK not in library path

**Solution:**
1. Verify Windows SDK installed
2. Restart Visual Studio
3. Clean and rebuild:
   ```bash
   cmake --build . --clean-first
   ```

---

## Runtime Errors

### Error: "The application was unable to start correctly (0xc000007b)"

**Cause:** 32-bit/64-bit mismatch or missing DLLs

**Solution:**
1. Verify building for x64:
   ```bash
   cmake -A x64 ..
   ```
2. Install Visual C++ Redistributable:
   - Download from Microsoft
   - Install both x86 and x64 versions

### Error: "MSVCP140.dll was not found"

**Cause:** Visual C++ Redistributable not installed

**Solution:**
Download and install:
- [Visual C++ Redistributable 2015-2022](https://aka.ms/vs/17/release/vc_redist.x64.exe)

### Error: "Administrator Required" dialog appears

**Cause:** Application needs admin privileges

**Solution:**
```bash
# PowerShell - Run as admin
Start-Process .\DriverMonitor.exe -Verb RunAs

# Or right-click executable → Run as Administrator
```

### Error: "Failed to initialize application"

**Possible Causes:**
1. DirectX 11 not available
2. ImGui not properly integrated
3. Missing DLLs

**Solutions:**

Check DirectX:
```cmd
dxdiag
# Look for DirectX version in System tab
```

Verify ImGui:
```bash
# Ensure imgui.ini can be created (write permissions)
cd build/bin/Release
icacls . /grant Users:F
```

Check Event Viewer:
```
Windows Logs → Application
Look for DriverMonitor errors
```

### Error: "No events appearing in log"

**Cause:** Filters blocking all events or not running as admin

**Solutions:**

1. **Verify admin:**
   - Check UAC shield on application
   - Restart as Administrator

2. **Disable filters:**
   - Uncheck "Ignore Windows Signed"
   - Uncheck "Ignore Microsoft"
   - Clear search box
   - Set filter to "All"

3. **Test with driver install:**
   ```cmd
   # As Administrator
   # Plug in USB device to trigger driver load
   ```

### Error: "Application crashes on startup"

**Debug Steps:**

1. **Enable debug build:**
   ```bash
   cmake --build . --config Debug
   ```

2. **Run in Visual Studio debugger:**
   - Open DriverMonitor.sln
   - Press F5
   - Note exception details

3. **Check logs:**
   ```bash
   type driver_monitor.log
   ```

4. **Check Windows Event Viewer:**
   ```
   Windows Logs → Application
   Filter by "Error"
   ```

---

## Performance Issues

### Issue: High CPU usage (>20%)

**Possible Causes:**
1. Too many events
2. Verbose mode enabled
3. Fast monitoring intervals

**Solutions:**

1. **Reduce max events:**
   ```json
   {
     "ui": {
       "maxEvents": 500
     }
   }
   ```

2. **Disable verbose mode:**
   ```json
   {
     "monitoring": {
       "verboseMode": false
     }
   }
   ```

3. **Enable filters:**
   - Check "Ignore Windows Signed"
   - Check "Ignore Microsoft"

### Issue: High memory usage (>200MB)

**Possible Causes:**
1. Too many events stored
2. Memory leak

**Solutions:**

1. **Reduce max events:**
   - Lower "Max Events" slider in Settings
   - Click "Clear Log" periodically

2. **Check for memory leak:**
   ```bash
   # Run for 1 hour and monitor Task Manager
   # Memory should stabilize, not continuously grow
   ```

### Issue: GUI freezes or stutters

**Possible Causes:**
1. Too many events rendered
2. Blocking operations on main thread
3. Slow signature verification

**Solutions:**

1. **Use filtering:**
   - Filter by type
   - Use search to reduce visible events

2. **Disable auto-scroll:**
   - Uncheck "Auto-scroll Log"

3. **Clear old events:**
   - Click "Clear Log" button

### Issue: Slow application startup

**Possible Causes:**
1. Initial driver scan
2. Large log file
3. Many whitelisted drivers

**Solutions:**

1. **Delete large log:**
   ```bash
   del driver_monitor.log
   ```

2. **Reduce whitelist:**
   ```json
   {
     "whitelist": []
   }
   ```

---

## Advanced Troubleshooting

### Enable Debug Output

Add to source files:
```cpp
#include <iostream>
std::cout << "Debug: Variable = " << variable << std::endl;
```

Build as console app temporarily:
```cmake
# Comment out in CMakeLists.txt:
# set_target_properties(DriverMonitor PROPERTIES WIN32_EXECUTABLE TRUE)
```

### Use Windows Debugger

```cmd
# Install Windows SDK
# Run with WinDbg:
"C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\windbg.exe" DriverMonitor.exe
```

### Check Dependencies

```cmd
# Use Dependency Walker
# Download from: http://www.dependencywalker.com/
depends.exe DriverMonitor.exe
```

### Memory Leak Detection

Add to Application.cpp:
```cpp
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// In Application destructor:
_CrtDumpMemoryLeaks();
```

---

## Getting More Help

If none of these solutions work:

1. **Gather information:**
   - Windows version (`winver`)
   - Visual Studio version
   - CMake version
   - Full error message
   - Screenshots
   - Log files

2. **Search existing issues:**
   - https://github.com/hz1901213/driver-monitor/issues

3. **Open new issue:**
   - Include all gathered information
   - Steps to reproduce
   - What you've already tried

4. **Community support:**
   - Check ImGui issues for GUI problems
   - Stack Overflow for Windows API issues
   - DirectX forums for rendering issues

---

## Quick Fixes Checklist

- [ ] Running as Administrator
- [ ] ImGui downloaded to libs/imgui/
- [ ] Windows SDK installed
- [ ] Visual Studio C++ workload installed
- [ ] Built for x64 architecture
- [ ] Clean rebuild (`cmake --build . --clean-first`)
- [ ] Visual C++ Redistributable installed
- [ ] DirectX 11 available (`dxdiag`)
- [ ] No antivirus blocking
- [ ] Sufficient disk space
- [ ] No special characters in path
- [ ] Latest Windows updates installed

---

**Still having issues?** Open an issue with full details: https://github.com/hz1901213/driver-monitor/issues
