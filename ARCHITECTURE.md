# Architecture Overview

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      Windows Driver Monitor                      │
│                          (DriverMonitor.exe)                     │
└─────────────────────────────────────────────────────────────────┘
                                  │
                    ┌─────────────┴──────────────┐
                    │                            │
          ┌─────────▼─────────┐      ┌──────────▼──────────┐
          │   GUI Layer       │      │  Monitoring Layer   │
          │   (ImGui + D3D11) │      │   (Background       │
          │                   │      │    Threads)         │
          └─────────┬─────────┘      └──────────┬──────────┘
                    │                           │
         ┌──────────┴───────────┐    ┌──────────┴──────────┐
         │                      │    │                     │
    ┌────▼────┐           ┌────▼────▼────┐           ┌───▼────┐
    │ Main    │           │ Event         │           │ Config │
    │ Window  │◄──────────┤ Manager       │──────────►│        │
    │         │           │ (Thread-Safe) │           │        │
    └────┬────┘           └────┬──────────┘           └────────┘
         │                     │
    ┌────┴─────────────────────┴────┐
    │                                │
┌───▼────┐  ┌─────┐  ┌─────┐  ┌────▼────┐
│Control │  │Stats│  │Event│  │Details  │
│Panel   │  │     │  │ Log │  │ Panel   │
└────────┘  └─────┘  └─────┘  └─────────┘
    │           │        │          │
    └───────────┴────────┴──────────┘
                 │
         ┌───────▼────────┐
         │ Settings Panel │
         │ Filter Panel   │
         └────────────────┘
```

## Component Hierarchy

```
Application (WinMain)
│
├── DirectX 11 Setup
│   ├── Device Creation
│   ├── Swap Chain
│   └── Render Target View
│
├── ImGui Initialization
│   ├── Context Creation
│   ├── Win32 Backend
│   ├── DirectX 11 Backend
│   └── Dark Theme Application
│
├── Core Components
│   ├── EventManager
│   │   ├── Event Queue (thread-safe)
│   │   ├── Statistics Tracking
│   │   └── Event Filtering
│   │
│   ├── Config
│   │   ├── JSON Parsing
│   │   ├── Configuration State
│   │   └── Whitelist Management
│   │
│   └── Utils
│       ├── Timestamp Generation
│       ├── Process Name Resolution
│       ├── Digital Signature Verification
│       └── Threat Assessment
│
├── DriverMonitor (Coordinator)
│   ├── Registry Monitor Thread
│   ├── FileSystem Monitor Thread
│   ├── WMI Monitor Thread
│   └── Event Processing Pipeline
│
└── MainWindow (GUI)
    ├── Control Panel
    ├── Statistics Panel
    ├── Settings Panel
    ├── Filter Panel
    ├── Event Log Panel
    └── Details Panel
```

## Data Flow

```
Driver Load Event
      │
      ▼
┌─────────────────────┐
│ Windows System      │
│ (Registry/FS/WMI)   │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Monitoring Thread   │
│ - Registry          │
│ - FileSystem        │
│ - WMI               │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ DriverMonitor       │
│ ProcessDriverEvent()│
└──────────┬──────────┘
           │
           ├──► Get Signature Info (Utils)
           ├──► Determine Event Type (Utils)
           ├──► Assess Threat Level (Utils)
           │
           ▼
┌─────────────────────┐
│ Filter Check        │
│ - Whitelist?        │
│ - Ignore MS?        │
│ - Ignore Windows?   │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ EventManager        │
│ AddEvent()          │
│ (Thread-Safe Queue) │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ GUI Update          │
│ (60 FPS)            │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ User Interface      │
│ - Event Log         │
│ - Statistics        │
│ - Details           │
└─────────────────────┘
```

## Threading Model

```
┌──────────────────────────────────────────────────────┐
│ Main Thread (GUI)                                     │
│                                                       │
│  ┌─────────────────────────────────────────┐        │
│  │ ImGui Render Loop (60 FPS)              │        │
│  │  - NewFrame()                            │        │
│  │  - MainWindow::Render()                 │        │
│  │  - RenderDrawData()                     │        │
│  │  - Present()                            │        │
│  └─────────────────────────────────────────┘        │
│                      │                               │
│                      ▼                               │
│            GetEvents() [Mutex Lock]                 │
│                      │                               │
└──────────────────────┼───────────────────────────────┘
                       │
                  ┌────▼────┐
                  │ Event   │ ◄───────┐
                  │ Manager │         │
                  │ (Mutex) │         │
                  └────▲────┘         │
                       │              │
         ┌─────────────┴─────────┬────┴──────┐
         │                       │           │
┌────────▼────────┐   ┌──────────▼──────┐   │
│ Registry        │   │ FileSystem      │   │
│ Monitor Thread  │   │ Monitor Thread  │   │
│                 │   │                 │   │
│ while(running)  │   │ while(running)  │   │
│   check()       │   │   check()       │   │
│   AddEvent()────┼───┤   AddEvent()────┼───┤
│   sleep(500ms)  │   │   sleep(1000ms) │   │
└─────────────────┘   └─────────────────┘   │
                                             │
                            ┌────────────────┘
                            │
                    ┌───────▼──────┐
                    │ WMI          │
                    │ Monitor      │
                    │ Thread       │
                    │              │
                    │ while(run)   │
                    │   check()    │
                    │   AddEvent() │
                    │   sleep(2s)  │
                    └──────────────┘
```

## Key Design Patterns

### 1. Producer-Consumer Pattern
- **Producers:** Monitoring threads (Registry, FileSystem, WMI)
- **Queue:** EventManager (thread-safe)
- **Consumer:** GUI thread (reads events for display)

### 2. Observer Pattern
- **Subject:** EventManager
- **Observers:** GUI panels (Statistics, Event Log)

### 3. Strategy Pattern
- **Context:** DriverMonitor
- **Strategies:** Different monitoring methods (Registry, FileSystem, WMI, ETW)

### 4. Singleton-like Pattern
- **EventManager:** Single instance shared across threads
- **Config:** Single configuration instance

### 5. MVC-inspired Pattern
- **Model:** EventManager, Config, DriverEvent
- **View:** GUI Panels (MainWindow)
- **Controller:** DriverMonitor, Application

## Thread Safety

### Protected Resources
1. **EventManager::m_events** (std::vector)
   - Protected by: std::mutex
   - Accessed by: All monitoring threads (write), GUI thread (read)

2. **Config::m_config** (MonitorConfig)
   - Protected by: Main thread only (GUI modifications)
   - Accessed by: Monitoring threads (read-only after load)

### Synchronization Points
```cpp
// Adding event (monitoring thread)
void EventManager::AddEvent(const DriverEvent& event) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.push_back(event);
    // Update statistics
}

// Reading events (GUI thread)
std::vector<DriverEvent> EventManager::GetEvents() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_events;  // Copy
}
```

## Memory Management

### Smart Pointers Usage
```cpp
Application:
  std::unique_ptr<EventManager> m_eventManager;
  std::unique_ptr<Config> m_config;
  std::unique_ptr<DriverMonitor> m_driverMonitor;
  std::unique_ptr<MainWindow> m_mainWindow;

DriverMonitor:
  std::unique_ptr<std::thread> m_registryThread;
  std::unique_ptr<std::thread> m_fileSystemThread;
  std::unique_ptr<std::thread> m_wmiThread;
```

### RAII Pattern
- DirectX resources (Device, Context, SwapChain, RenderTarget)
- Threads (join on destruction)
- File handles (automatic close)
- Windows API handles (CloseHandle)

## Performance Considerations

### Event Limiting
```
Max Events: 1000 (configurable)
│
├── New event arrives
│   ├── If count > max
│   │   └── Remove oldest event
│   │       └── Update statistics
│   └── Add new event
│       └── Update statistics
```

### Rendering Optimization
- **VSync enabled:** 60 FPS cap (prevents unnecessary rendering)
- **ImGuiListClipper:** Only render visible rows in event log
- **Minimal redraws:** ImGui only updates changed elements

### Monitoring Intervals
- Registry: 500ms
- FileSystem: 1000ms
- WMI: 2000ms

Prevents excessive CPU usage while maintaining responsiveness.

## Configuration Flow

```
Startup
   │
   ▼
Load config.json
   │
   ├──► Parse JSON
   ├──► Set Config values
   └──► Apply to EventManager
   │
Runtime
   │
   ├──► User changes settings
   │    └──► Update Config in memory
   │
   └──► Click "Save Config"
        └──► Write config.json
        
Shutdown
   │
   └──► Auto-save config.json
```

## Error Handling Strategy

### Graceful Degradation
```
Monitoring Method Fails
   │
   ├──► Log error
   ├──► Continue with other methods
   └──► Display status in GUI
   
Example:
  Registry Monitor fails
    └──► FileSystem and WMI continue working
```

### User Notifications
- **Critical:** MessageBox (blocking)
- **Warning:** Status text in GUI (non-blocking)
- **Info:** Event log entry

## Security Model

### Privilege Requirements
```
Application Start
   │
   ├──► Check if Admin
   │    ├──► Yes: Continue
   │    └──► No: Show error, exit
   │
   └──► Start monitoring
        └──► Access privileged resources
            ├── Registry: HKLM\SYSTEM
            ├── Files: System32\drivers
            └── WMI: System queries
```

### Signature Verification
```
Driver Event
   │
   └──► GetSignerInfo()
        └──► WinVerifyTrust()
             ├──► Signed by Microsoft → Low threat
             ├──► Signed by other → Medium threat
             └──► Not signed → High threat
```

## File I/O Operations

### Configuration
```
config.json
   │
   ├── Read: Application::Initialize()
   ├── Write: User clicks "Save Config"
   └── Auto-save: Application::Shutdown()
```

### Logging
```
driver_monitor.log
   │
   ├── Append: Each event (if logging enabled)
   ├── Format: [timestamp] [type] driver - method - signer
   └── Rotation: Manual (max size configurable)
```

### Export
```
driver_monitor_export.txt
   │
   └── Write: User clicks "Export Logs"
       └── Format: Full event details with headers
```

## Extensibility Points

### Adding New Monitoring Method
1. Create new Monitor class (e.g., `ProcessMonitor.h/cpp`)
2. Implement `CheckForNewDrivers()` method
3. Add thread in `DriverMonitor.cpp`
4. Add method-specific event type

### Adding GUI Panel
1. Add rendering function in `MainWindow.cpp`
2. Call from `MainWindow::Render()`
3. Add UI state variables if needed

### Adding Event Field
1. Update `DriverEvent` struct in `Utils.h`
2. Update event processing in `DriverMonitor.cpp`
3. Update GUI display in `MainWindow.cpp`
4. Update export format

## Future Architecture Enhancements

### Planned
- [ ] Full ETW implementation (kernel event tracing)
- [ ] Kernel driver for actual blocking
- [ ] Database backend (SQLite) for history
- [ ] Network communication (multi-computer monitoring)
- [ ] Plugin system for custom monitors

### Considered
- [ ] Machine learning threat detection
- [ ] Behavioral analysis
- [ ] Process correlation
- [ ] Network activity monitoring
