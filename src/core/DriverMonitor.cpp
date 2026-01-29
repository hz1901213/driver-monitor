#include "DriverMonitor.h"
#include "../monitoring/ETWConsumer.h"
#include "../monitoring/RegistryMonitor.h"
#include "../monitoring/FileSystemMonitor.h"
#include "../monitoring/WMIMonitor.h"
#include <Windows.h>
#include <fstream>

namespace DriverMonitor {

DriverMonitor::DriverMonitor(EventManager* eventManager, Config* config)
    : m_eventManager(eventManager)
    , m_config(config)
    , m_isMonitoring(false) {
}

DriverMonitor::~DriverMonitor() {
    Stop();
}

bool DriverMonitor::Start() {
    if (m_isMonitoring) {
        return false; // Already monitoring
    }
    
    m_isMonitoring = true;
    m_startTime = std::chrono::steady_clock::now();
    
    // Start monitoring threads
    try {
        m_registryThread = std::make_unique<std::thread>(&DriverMonitor::RegistryMonitoringThread, this);
        m_fileSystemThread = std::make_unique<std::thread>(&DriverMonitor::FileSystemMonitoringThread, this);
        m_wmiThread = std::make_unique<std::thread>(&DriverMonitor::WMIMonitoringThread, this);
    } catch (...) {
        m_isMonitoring = false;
        return false;
    }
    
    return true;
}

void DriverMonitor::Stop() {
    if (!m_isMonitoring) {
        return;
    }
    
    m_isMonitoring = false;
    
    // Wait for threads to finish
    if (m_registryThread && m_registryThread->joinable()) {
        m_registryThread->join();
    }
    if (m_fileSystemThread && m_fileSystemThread->joinable()) {
        m_fileSystemThread->join();
    }
    if (m_wmiThread && m_wmiThread->joinable()) {
        m_wmiThread->join();
    }
}

int DriverMonitor::GetUptimeSeconds() const {
    if (!m_isMonitoring) {
        return 0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
    return static_cast<int>(duration.count());
}

void DriverMonitor::RegistryMonitoringThread() {
    RegistryMonitor monitor;
    
    while (m_isMonitoring) {
        DriverEvent event = monitor.CheckForNewDrivers();
        
        if (!event.driverName.empty()) {
            ProcessDriverEvent(event);
        }
        
        // Sleep to avoid excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void DriverMonitor::FileSystemMonitoringThread() {
    FileSystemMonitor monitor;
    
    while (m_isMonitoring) {
        DriverEvent event = monitor.CheckForNewDrivers();
        
        if (!event.driverName.empty()) {
            ProcessDriverEvent(event);
        }
        
        // Sleep to avoid excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void DriverMonitor::WMIMonitoringThread() {
    WMIMonitor monitor;
    
    while (m_isMonitoring) {
        DriverEvent event = monitor.CheckForNewDrivers();
        
        if (!event.driverName.empty()) {
            ProcessDriverEvent(event);
        }
        
        // Sleep to avoid excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}

void DriverMonitor::ProcessDriverEvent(DriverEvent& event) {
    // Set timestamp
    event.timestamp = Utils::GetTimestamp();
    
    // Get signer info if path is available
    if (!event.installPath.empty()) {
        event.signerInfo = Utils::GetSignerInfo(event.installPath);
    }
    
    // Determine event type
    event.eventType = Utils::DetermineEventType(event.signerInfo, event.loadingMethod);
    
    // Assess threat level
    event.threatLevel = Utils::AssessThreatLevel(event);
    
    // Check if should be filtered
    if (ShouldFilter(event)) {
        if (m_config->GetConfig().verboseMode) {
            // Log filtered events in verbose mode
            LogEvent(event);
        }
        return;
    }
    
    // Add to event manager
    m_eventManager->AddEvent(event);
    
    // Log to file
    if (m_config->GetConfig().loggingEnabled) {
        LogEvent(event);
    }
    
    // Play sound alert for critical events
    if (m_config->GetConfig().playSound && event.eventType == EventType::Suspicious) {
        MessageBeep(MB_ICONWARNING);
    }
}

bool DriverMonitor::ShouldFilter(const DriverEvent& event) const {
    const auto& config = m_config->GetConfig();
    
    // Check whitelist
    if (m_config->IsWhitelisted(event.driverName)) {
        return true;
    }
    
    // Check ignore Windows signed
    if (config.ignoreWindowsSigned && Utils::IsWindowsSigned(event.signerInfo)) {
        return true;
    }
    
    // Check ignore Microsoft
    if (config.ignoreMicrosoft && Utils::IsMicrosoftSigned(event.signerInfo)) {
        return true;
    }
    
    return false;
}

void DriverMonitor::LogEvent(const DriverEvent& event) {
    const auto& config = m_config->GetConfig();
    
    if (!config.loggingEnabled) {
        return;
    }
    
    std::ofstream logFile(config.logFile, std::ios::app);
    if (!logFile.is_open()) {
        return;
    }
    
    std::string eventTypeStr;
    switch (event.eventType) {
        case EventType::Signed: eventTypeStr = "SIGNED"; break;
        case EventType::Unsigned: eventTypeStr = "UNSIGNED"; break;
        case EventType::Suspicious: eventTypeStr = "SUSPICIOUS"; break;
    }
    
    logFile << event.timestamp << " [" << eventTypeStr << "] "
            << event.driverName << " - "
            << event.loadingMethod << " - "
            << event.signerInfo << "\n";
    
    logFile.close();
}

} // namespace DriverMonitor
