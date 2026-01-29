# Quick Start Guide - Windows Driver Monitor

## Installation in 5 Minutes

### 1. Prerequisites Check
```powershell
# Check Windows version (must be Windows 10/11)
winver

# Check if Visual Studio is installed
where msbuild
```

### 2. Download ImGui
```bash
cd driver-monitor
git clone https://github.com/ocornut/imgui.git libs/imgui
```

### 3. Build
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 4. Run
```bash
cd bin\Release
# Right-click DriverMonitor.exe → Run as Administrator
```

## Common Commands

### Building
```bash
# Clean build
cmake --build . --clean-first

# Debug build
cmake --build . --config Debug

# Release build
cmake --build . --config Release
```

### Running
```bash
# From build directory
.\bin\Release\DriverMonitor.exe

# Or with runas
runas /user:Administrator .\bin\Release\DriverMonitor.exe
```

## GUI Quick Reference

### Keyboard Shortcuts (Planned)
- **Ctrl+F** - Focus search
- **Ctrl+S** - Save config
- **Ctrl+L** - Clear log
- **Ctrl+E** - Export logs

### Mouse Actions
- **Click event** - Select and show details
- **Double-click** - (Future: Show popup with full info)
- **Right-click** - Context menu (Copy, Whitelist, View File)

### Color Coding
- 🟢 **Green (Teal)** - Signed by Microsoft (safe)
- 🟡 **Orange** - Unsigned or non-Microsoft signed (warning)
- 🔴 **Red** - Suspicious loading method (critical)

## Filters Explained

### Ignore Windows Signed
When **ON**: Hides drivers signed by Microsoft Windows
When **OFF**: Shows all drivers

### Ignore Microsoft
When **ON**: Hides all Microsoft-signed drivers
When **OFF**: Shows all drivers

### Block Unsigned (Not Implemented)
Requires kernel driver to actually block loading.
Currently just shows warning.

### Verbose Mode
When **ON**: Logs filtered events too
When **OFF**: Only logs visible events

## Troubleshooting Quick Fixes

### "Administrator Required"
```bash
# PowerShell - Run as admin
Start-Process .\DriverMonitor.exe -Verb RunAs
```

### "Failed to initialize"
1. Reinstall Visual Studio C++ Redistributable
2. Update Windows
3. Check DirectX 11 availability: `dxdiag`

### "No events appearing"
1. Verify Status shows "ON"
2. Disable all filters temporarily
3. Check Windows Event Viewer for actual driver loads
4. Try verbose mode

### Build fails with ImGui errors
```bash
# Verify ImGui structure
dir libs\imgui\imgui.h
dir libs\imgui\backends\imgui_impl_dx11.h

# If missing, re-clone
rmdir /s libs\imgui
git clone https://github.com/ocornut/imgui.git libs/imgui
```

## Configuration Tips

### Recommended Settings for Production Use
```json
{
  "monitoring": {
    "ignoreWindowsSigned": true,    // Reduce noise
    "ignoreMicrosoft": true,         // Focus on third-party
    "verboseMode": false             // Better performance
  },
  "ui": {
    "autoScroll": true,              // Always see latest
    "maxEvents": 1000                // Keep memory low
  }
}
```

### Recommended Settings for Testing/Development
```json
{
  "monitoring": {
    "ignoreWindowsSigned": false,   // See everything
    "ignoreMicrosoft": false,        // See everything
    "verboseMode": true              // Debug info
  },
  "ui": {
    "autoScroll": true,
    "maxEvents": 5000                // Keep more history
  }
}
```

## Performance Tips

1. **Use filtering** - Reduce displayed events
2. **Lower max events** - Keep under 1000 for best performance
3. **Disable verbose mode** - Reduces CPU usage
4. **Close Details panel** - When not needed

## Testing the Application

### Method 1: Windows Update
1. Start monitoring
2. Check for Windows updates
3. Install driver updates
4. Observe events

### Method 2: USB Device
1. Start monitoring
2. Plug in USB device
3. New driver events should appear

### Method 3: Create Test Service (Advanced)
```cmd
# As Administrator
sc create testdrv type=kernel binPath=C:\Windows\System32\drivers\null.sys
sc start testdrv
# Should appear in event log
sc delete testdrv
```

## File Locations

- **Executable:** `build/bin/Release/DriverMonitor.exe`
- **Configuration:** `build/bin/Release/config.json`
- **Log file:** `build/bin/Release/driver_monitor.log`
- **ImGui settings:** `build/bin/Release/imgui.ini`

## Default Log Format
```
[15:32:45] [SIGNED] driver.sys - Registry - Service Installation - Signed by Microsoft Corporation
[15:33:12] [UNSIGNED] test.sys - File System - New Driver File - Not Signed
[15:33:45] [SUSPICIOUS] unknown.sys - Manual Map Detection - Not Signed
```

## Getting Help

1. Check README.md for comprehensive documentation
2. Review logs: `driver_monitor.log`
3. Check Windows Event Viewer: Applications and Services → Microsoft → Windows → DriverMonitor
4. Open issue on GitHub with:
   - Windows version
   - Visual Studio version
   - Full error message
   - Steps to reproduce

## Safety Notes

⚠️ **Always:**
- Run as Administrator
- Keep Windows updated
- Monitor system performance
- Review events regularly

⚠️ **Never:**
- Block all drivers (system will crash)
- Delete driver files without knowing what they are
- Whitelist unknown suspicious drivers

## Next Steps After Installation

1. ✅ Start monitoring
2. ✅ Configure filters for your needs
3. ✅ Add known-good drivers to whitelist
4. ✅ Set up log rotation if needed
5. ✅ Monitor for a few days to establish baseline
6. ✅ Review suspicious events
7. ✅ Export logs for analysis

---

**Pro Tip:** Run for 24 hours with verbose mode OFF and all filters ON to establish a baseline of normal activity on your system. Then review what gets through and adjust settings accordingly.
