# ImGui Setup Instructions

This document provides detailed instructions for downloading and integrating Dear ImGui into the Driver Monitor project.

## What is ImGui?

Dear ImGui is a bloat-free graphical user interface library for C++. It's designed to enable fast iterations and empower programmers to create content creation tools and visualization/debug tools.

- **GitHub:** https://github.com/ocornut/imgui
- **Documentation:** https://github.com/ocornut/imgui/wiki
- **License:** MIT

## Why Download Separately?

ImGui is not included in this repository because:
1. It's maintained by a third party (Omar Cornut)
2. It gets frequent updates
3. Users may want specific versions
4. Keeps repository size small

## Download Options

### Option 1: Git Clone (Recommended)

```bash
cd driver-monitor
git clone https://github.com/ocornut/imgui.git libs/imgui
```

**Advantages:**
- Easy to update (`git pull`)
- Gets latest version
- Includes all files

### Option 2: Download ZIP

1. Go to: https://github.com/ocornut/imgui
2. Click "Code" → "Download ZIP"
3. Extract to `driver-monitor/libs/imgui/`
4. Rename folder from `imgui-master` to `imgui`

### Option 3: Specific Version

```bash
cd driver-monitor
git clone --branch v1.89.9 https://github.com/ocornut/imgui.git libs/imgui
```

Replace `v1.89.9` with desired version from: https://github.com/ocornut/imgui/releases

## Verify Installation

After downloading, verify the directory structure:

```
driver-monitor/
├── libs/
│   └── imgui/
│       ├── imgui.h           ✓ Core header
│       ├── imgui.cpp         ✓ Core implementation
│       ├── imgui_draw.cpp    ✓ Drawing
│       ├── imgui_widgets.cpp ✓ Widgets
│       ├── imgui_tables.cpp  ✓ Tables
│       ├── imgui_demo.cpp    ✓ Demo (optional)
│       ├── imgui_internal.h  ✓ Internal header
│       ├── imconfig.h        ✓ Configuration
│       ├── imstb_rectpack.h  ✓ Rect packing
│       ├── imstb_textedit.h  ✓ Text edit
│       ├── imstb_truetype.h  ✓ Font
│       └── backends/
│           ├── imgui_impl_win32.h   ✓ Windows backend
│           ├── imgui_impl_win32.cpp ✓ Windows backend
│           ├── imgui_impl_dx11.h    ✓ DirectX 11 backend
│           └── imgui_impl_dx11.cpp  ✓ DirectX 11 backend
```

### Quick Verification Script

**Windows PowerShell:**
```powershell
# Run from driver-monitor directory
$files = @(
    "libs/imgui/imgui.h",
    "libs/imgui/imgui.cpp",
    "libs/imgui/imgui_draw.cpp",
    "libs/imgui/imgui_widgets.cpp",
    "libs/imgui/imgui_tables.cpp",
    "libs/imgui/backends/imgui_impl_win32.h",
    "libs/imgui/backends/imgui_impl_win32.cpp",
    "libs/imgui/backends/imgui_impl_dx11.h",
    "libs/imgui/backends/imgui_impl_dx11.cpp"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "✓ $file" -ForegroundColor Green
    } else {
        Write-Host "✗ $file - MISSING!" -ForegroundColor Red
    }
}
```

**Linux/macOS/Git Bash:**
```bash
#!/bin/bash
files=(
    "libs/imgui/imgui.h"
    "libs/imgui/imgui.cpp"
    "libs/imgui/imgui_draw.cpp"
    "libs/imgui/imgui_widgets.cpp"
    "libs/imgui/imgui_tables.cpp"
    "libs/imgui/backends/imgui_impl_win32.h"
    "libs/imgui/backends/imgui_impl_win32.cpp"
    "libs/imgui/backends/imgui_impl_dx11.h"
    "libs/imgui/backends/imgui_impl_dx11.cpp"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file"
    else
        echo "✗ $file - MISSING!"
    fi
done
```

## CMake Integration

The project's `CMakeLists.txt` already references ImGui files:

```cmake
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)
```

No changes needed to CMakeLists.txt after downloading ImGui!

## Common Issues

### Issue: "Cannot find imgui.h"

**Cause:** ImGui not in correct location

**Solution:**
```bash
# Check if directory exists
ls libs/imgui/imgui.h

# If missing, download again
git clone https://github.com/ocornut/imgui.git libs/imgui
```

### Issue: "imgui-master" folder instead of "imgui"

**Cause:** Downloaded ZIP extracts to `imgui-master`

**Solution:**
```bash
# Windows
move libs\imgui-master libs\imgui

# Linux/macOS
mv libs/imgui-master libs/imgui
```

### Issue: CMake can't find backends

**Cause:** Old ImGui version or incorrect structure

**Solution:**
1. Download latest ImGui
2. Ensure `backends/` subfolder exists
3. Check for `imgui_impl_dx11.cpp` and `imgui_impl_win32.cpp`

### Issue: Build errors with ImGui

**Cause:** Version incompatibility

**Solution:**
```bash
# Try specific known-good version
cd libs
rm -rf imgui
git clone --branch v1.89.9 https://github.com/ocornut/imgui.git imgui
```

## Updating ImGui

To update to latest version:

```bash
cd libs/imgui
git pull origin master
```

Then rebuild the project:
```bash
cd ../../build
cmake --build . --clean-first
```

## Alternative: Manual Download

If you can't use Git:

1. Visit: https://github.com/ocornut/imgui/archive/refs/heads/master.zip
2. Download and extract
3. Copy contents to `libs/imgui/`:
   ```
   driver-monitor/libs/imgui/
   ├── *.h
   ├── *.cpp
   └── backends/
       ├── imgui_impl_win32.*
       └── imgui_impl_dx11.*
   ```

## Files Required by This Project

**Minimum Required Files:**
- `imgui.h`
- `imgui.cpp`
- `imgui_draw.cpp`
- `imgui_widgets.cpp`
- `imgui_tables.cpp`
- `imgui_internal.h`
- `imconfig.h`
- `imstb_rectpack.h`
- `imstb_textedit.h`
- `imstb_truetype.h`
- `backends/imgui_impl_win32.h`
- `backends/imgui_impl_win32.cpp`
- `backends/imgui_impl_dx11.h`
- `backends/imgui_impl_dx11.cpp`

**Optional Files (not used but can be present):**
- `imgui_demo.cpp` - Demo window (useful for learning)
- `examples/*` - Example applications
- `docs/*` - Documentation
- Other backend files (OpenGL, Vulkan, etc.)

## License Compatibility

Dear ImGui is licensed under the **MIT License**, which is compatible with most projects:

```
The MIT License (MIT)

Copyright (c) 2014-2024 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

## Testing ImGui Installation

After downloading, try building:

```bash
cd build
cmake ..
cmake --build .
```

If successful, you should see:
```
Building ImGui...
Building Application...
Linking DriverMonitor...
Build succeeded.
```

## Getting Help

- **ImGui Issues:** https://github.com/ocornut/imgui/issues
- **ImGui Wiki:** https://github.com/ocornut/imgui/wiki
- **This Project Issues:** https://github.com/hz1901213/driver-monitor/issues

## Summary

1. ✅ Download ImGui to `libs/imgui/`
2. ✅ Verify all required files present
3. ✅ Run CMake
4. ✅ Build project
5. ✅ Run application

**That's it!** CMakeLists.txt handles the rest.
