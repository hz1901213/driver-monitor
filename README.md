# Windows Driver Monitor v2.0

A comprehensive C++ Windows driver monitoring application with a modern, functional ImGui-based GUI interface. Monitor driver loads in real-time using multiple detection methods including Registry monitoring, File System watching, and WMI queries.

![License](https://img.shields.io/badge/license-Educational-blue)
![Platform](https://img.shields.io/badge/platform-Windows-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)

## 🌟 Features

### Core Monitoring Capabilities
- **Multi-method driver detection:**
  - Registry monitoring (HKLM\SYSTEM\CurrentControlSet\Services)
  - File system monitoring (System32\drivers directory)
  - WMI event subscriptions (Win32_SystemDriver)
  - ETW (Event Tracing for Windows) support (placeholder)
  
### Modern GUI Interface
- **ImGui** with **DirectX 11** backend for smooth 60 FPS rendering
- Professional dark theme with color-coded events
- Dockable panels for customizable layout
- Real-time event log with auto-scroll
- Detailed event information viewer
- Comprehensive statistics dashboard

### Advanced Features
- ✅ **Ignore Windows-signed drivers** - Filter out Microsoft signed drivers
- ✅ **Ignore Microsoft drivers** - Filter drivers signed by Microsoft
- ✅ **Whitelist management** - User-configurable driver whitelist
- ✅ **Search and filter** - Find specific drivers quickly
- ✅ **Event type filtering** - Show All/Signed/Unsigned/Suspicious
- ✅ **Color-coded alerts** - Green (signed), Yellow (unsigned), Red (suspicious)
- ✅ **Export logs** - Save events to text file
- ✅ **Sound alerts** - Audio notification for critical events
- ✅ **Configuration persistence** - Settings saved to JSON
- ✅ **Context menus** - Right-click for quick actions
- ✅ **Clipboard integration** - Copy event details

## 📋 Prerequisites

### Required Software
- **Windows 10/11** (x64)
- **Visual Studio 2019 or later** with C++ Desktop Development workload
- **Windows SDK 10** (10.0.19041.0 or later)
- **CMake 3.15+**
- **Git** (for cloning)

### Required Libraries
- **DirectX 11** (included in Windows SDK)
- **Dear ImGui** (must be downloaded separately)

## 🚀 Installation

### Step 1: Clone Repository

```bash
git clone https://github.com/hz1901213/driver-monitor.git
cd driver-monitor
```

### Step 2: Download and Install Dear ImGui

1. Download ImGui from the official repository:
   ```bash
   git clone https://github.com/ocornut/imgui.git libs/imgui
   ```

2. Or download the latest release from: https://github.com/ocornut/imgui/releases

3. Extract to `libs/imgui/` directory so the structure looks like:
   ```
   driver-monitor/
   ├── libs/
   │   └── imgui/
   │       ├── imgui.h
   │       ├── imgui.cpp
   │       ├── imgui_draw.cpp
   │       ├── imgui_widgets.cpp
   │       ├── imgui_tables.cpp
   │       └── backends/
   │           ├── imgui_impl_dx11.h
   │           ├── imgui_impl_dx11.cpp
   │           ├── imgui_impl_win32.h
   │           └── imgui_impl_win32.cpp
   ```

### Step 3: Build with CMake

#### Option A: Using CMake GUI
1. Open CMake GUI
2. Set source directory to `driver-monitor`
3. Set build directory to `driver-monitor/build`
4. Click "Configure" and select your Visual Studio version
5. Click "Generate"
6. Click "Open Project" to open in Visual Studio
7. Build the solution (F7 or Build → Build Solution)

#### Option B: Using Command Line
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Option C: Using Visual Studio Developer Command Prompt
```cmd
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
msbuild DriverMonitor.sln /p:Configuration=Release
```

### Step 4: Run the Application

**Important:** The application requires **Administrator privileges** to monitor drivers.

1. Navigate to the build output directory: `build/bin/Release/`
2. Right-click `DriverMonitor.exe` and select "Run as Administrator"
3. Or from command line:
   ```cmd
   cd build\bin\Release
   DriverMonitor.exe
   ```

## 📖 Usage Guide

### Getting Started

1. **Launch as Administrator** - Required for driver monitoring
2. **Click "Start Monitoring"** - Begin real-time driver detection
3. **View events** - New driver loads appear in the Event Log panel
4. **Click an event** - View detailed information in the Details panel
5. **Configure settings** - Adjust filters and options in Settings panel

### GUI Overview

#### Control Panel
- **Status Indicator** - Shows monitoring state (ON/OFF)
- **Start/Stop Button** - Toggle monitoring
- **Save Config** - Save current settings
- **Export Logs** - Export all events to text file

#### Statistics Panel
- **Drivers Detected** - Total count
- **Signed** - Microsoft signed drivers (green)
- **Unsigned** - Not signed drivers (yellow)
- **Suspicious** - Potentially dangerous (red)
- **Uptime** - Session duration (HH:MM:SS)

#### Settings Panel
- **Monitoring Options**
  - ☑ Ignore Windows Signed Drivers
  - ☑ Ignore Microsoft Drivers
  - ☐ Block Unsigned Drivers
  - ☐ Verbose Mode
- **Alert Options**
  - ☑ Play Alert Sound
  - ☑ Show Notifications
- **UI Options**
  - ☑ Auto-scroll Log
  - Max Events slider
- **Logging Options**
  - ☑ Enable Logging
  - Log file path
- **Whitelist Management**
  - View and remove whitelisted drivers

#### Filter Panel
- **Show dropdown** - Filter by type (All/Signed/Unsigned/Suspicious)
- **Search box** - Find drivers by name or path
- **Clear button** - Reset search

#### Event Log Panel
- **Real-time event display** with columns:
  - Time - Timestamp [HH:MM:SS]
  - Status - OK/WARN/CRIT with color
  - Driver - Driver file name
  - Method - Detection method used
  - Signer - Digital signature info
- **Right-click menu:**
  - Copy Details
  - Add to Whitelist
- **Clear Log button** - Remove all events

#### Event Details Panel
- **Driver Name** - Full name
- **Path** - Installation path
- **Method** - How detected
- **Initiated By** - Process that loaded (PID)
- **Signer** - Digital signature status
- **Threat Level** - Low/Medium/High assessment
- **Action Buttons:**
  - Copy Details - To clipboard
  - Add to Whitelist - Trust this driver
  - View File Location - Open in Explorer

### Configuration File

Settings are saved to `config.json` in the application directory:

```json
{
  "monitoring": {
    "ignoreWindowsSigned": true,
    "ignoreMicrosoft": true,
    "blockUnsigned": false,
    "verboseMode": false
  },
  "alerts": {
    "playSound": true,
    "showNotifications": true
  },
  "ui": {
    "autoScroll": true,
    "maxEvents": 1000
  },
  "logging": {
    "enabled": true,
    "logFile": "driver_monitor.log",
    "maxLogSize": 10485760
  },
  "whitelist": []
}
```

### Keyboard Shortcuts
- **Ctrl+F** - Focus search box (if implemented)
- **Ctrl+S** - Save configuration (if implemented)
- **Ctrl+C** - Copy selected event (if implemented)

## 🛠️ Troubleshooting

### Common Issues

#### "Administrator Required" Error
- **Solution:** Right-click the executable and select "Run as Administrator"

#### "Failed to initialize application"
- **Cause:** DirectX 11 not available or ImGui not properly installed
- **Solution:** 
  1. Verify Windows SDK is installed
  2. Check ImGui files are in `libs/imgui/` directory
  3. Rebuild the project

#### "DirectX 11 not found" during build
- **Solution:** Install Windows SDK from Visual Studio Installer

#### No events appearing
- **Solution:** 
  1. Ensure running as Administrator
  2. Check if monitoring is started (Status: ON)
  3. Verify filters are not blocking all events
  4. Try installing a test driver to trigger detection

#### High CPU usage
- **Normal:** 5-15% during active monitoring
- **If higher:** Check verbose mode is OFF

### Build Errors

#### "Cannot find imgui.h"
- **Solution:** Verify ImGui is in `libs/imgui/` directory

#### "LNK2019: unresolved external symbol"
- **Solution:** 
  1. Clean and rebuild: `cmake --build . --clean-first`
  2. Verify all ImGui .cpp files are in the directory

#### CMake errors about DirectX
- **Solution:** Install Windows SDK via Visual Studio Installer

## 🔒 Security Notes

### What It Can Detect
- ✅ Drivers loaded via normal Windows APIs
- ✅ Drivers registered in Services registry
- ✅ Driver files added to system directories
- ✅ Drivers loaded via WMI

### Limitations
- ❌ Cannot detect sophisticated rootkits
- ❌ Cannot block kernel-level driver loading (requires kernel driver)
- ❌ May miss manual mapping techniques
- ❌ ETW implementation is placeholder only
- ❌ Requires Administrator privileges (UAC prompt)

### Privacy
- All monitoring is local - no network activity
- No data is sent to external servers
- Logs are stored locally only

## 📊 Performance

### Resource Usage
- **Memory:** < 100 MB typical, < 200 MB with 1000 events
- **CPU:** < 5% idle, < 15% during active monitoring
- **GPU:** Minimal (DirectX 11 for UI only)
- **Disk:** Log files grow as configured (default 10 MB max)

### Performance Targets
- **GUI Refresh:** 60 FPS (vsync enabled)
- **Event Processing:** < 10ms latency
- **Startup Time:** < 3 seconds

## 🧪 Testing

### Manual Testing
1. Start the application as Administrator
2. Click "Start Monitoring"
3. Install a test driver (or use Windows Update)
4. Verify event appears in log
5. Test filters, search, and settings
6. Export logs and verify output

### Test Driver Installation (Advanced)
```cmd
# Create a test driver service (requires admin)
sc create testdriver type= kernel binPath= C:\Windows\System32\drivers\testdriver.sys
```

## 📝 Project Structure

```
driver-monitor/
├── src/
│   ├── main.cpp                    # Entry point
│   ├── Application.h/cpp           # Main app with D3D11
│   ├── core/
│   │   ├── Utils.h/cpp            # Utilities
│   │   ├── Config.h/cpp           # Configuration
│   │   ├── EventManager.h/cpp     # Event queue
│   │   └── DriverMonitor.h/cpp    # Core engine
│   ├── monitoring/
│   │   ├── ETWConsumer.h/cpp      # ETW (placeholder)
│   │   ├── RegistryMonitor.h/cpp  # Registry monitoring
│   │   ├── FileSystemMonitor.h/cpp # File system watching
│   │   └── WMIMonitor.h/cpp       # WMI queries
│   └── gui/
│       └── MainWindow.h/cpp       # GUI (all panels)
├── libs/
│   └── imgui/                     # Dear ImGui (download separately)
├── CMakeLists.txt                 # Build configuration
├── config.json                    # Default config
├── .gitignore                     # Git ignore rules
└── README.md                      # This file
```

## 🤝 Contributing

This is an educational project. Contributions are welcome for:
- Bug fixes
- Documentation improvements
- Feature enhancements
- Code quality improvements

## 📜 License

**Educational Use Only**

This project is provided for educational and research purposes. The author is not responsible for any misuse or damage caused by this software.

### Disclaimer
- Use at your own risk
- May not detect all driver loading methods
- Not suitable for production security monitoring
- Administrator privileges required

## 🔗 References

- [Dear ImGui](https://github.com/ocornut/imgui) - GUI library
- [Windows Driver Development](https://docs.microsoft.com/en-us/windows-hardware/drivers/)
- [DirectX 11 Documentation](https://docs.microsoft.com/en-us/windows/win32/direct3d11/atoc-dx-graphics-direct3d-11)

## 👨‍💻 Author

Created as a comprehensive driver monitoring tool with modern GUI.

## 🐛 Known Issues

1. ETW consumer is placeholder only (not fully implemented)
2. Manual mapping detection is basic heuristic
3. Block unsigned drivers option has no effect (requires kernel driver)
4. No multi-language support
5. Windows-only (no cross-platform support)

## 📅 Version History

### v2.0 (Current)
- Modern ImGui-based GUI with DirectX 11
- Real-time driver monitoring
- Comprehensive filtering and search
- Whitelist management
- Statistics dashboard
- Event details viewer
- Configuration persistence
- Export functionality

### v1.0 (Legacy)
- Console-based monitoring
- Basic ETW/Registry/WMI detection

## 🎯 Future Enhancements

- [ ] Full ETW implementation
- [ ] Advanced manual mapping detection
- [ ] Kernel driver for blocking capabilities
- [ ] Database backend for event history
- [ ] Network activity correlation
- [ ] Multi-computer monitoring
- [ ] Email/SMS alerts
- [ ] Web dashboard
- [ ] Machine learning threat detection

---

**⚠️ Important:** Always run as Administrator for proper functionality!

For issues, questions, or suggestions, please open an issue on GitHub.