#pragma once

#include "EventManager.h"
#include "Config.h"
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace DriverMonitor {

// Forward declarations for monitoring components
class ETWConsumer;
class RegistryMonitor;
class FileSystemMonitor;
class WMIMonitor;

class DriverMonitor {
public:
    DriverMonitor(EventManager* eventManager, Config* config);
    ~DriverMonitor();
    
    // Start monitoring
    bool Start();
    
    // Stop monitoring
    void Stop();
    
    // Check if monitoring is active
    bool IsMonitoring() const { return m_isMonitoring; }
    
    // Get uptime in seconds
    int GetUptimeSeconds() const;
    
private:
    EventManager* m_eventManager;
    Config* m_config;
    
    std::atomic<bool> m_isMonitoring;
    std::chrono::steady_clock::time_point m_startTime;
    
    // Monitoring threads
    std::unique_ptr<std::thread> m_registryThread;
    std::unique_ptr<std::thread> m_fileSystemThread;
    std::unique_ptr<std::thread> m_wmiThread;
    
    // Monitoring methods
    void RegistryMonitoringThread();
    void FileSystemMonitoringThread();
    void WMIMonitoringThread();
    
    // Process detected driver
    void ProcessDriverEvent(DriverEvent& event);
    
    // Check if event should be filtered
    bool ShouldFilter(const DriverEvent& event) const;
    
    // Log event to file
    void LogEvent(const DriverEvent& event);
};

} // namespace DriverMonitor
