#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace DriverMonitor {

// Threat level assessment
enum class ThreatLevel {
    Low,
    Medium,
    High
};

// Event type for color coding
enum class EventType {
    Signed,      // Green - Microsoft signed
    Unsigned,    // Yellow - Not signed
    Suspicious   // Red - Potentially dangerous
};

// Driver event structure
struct DriverEvent {
    std::string driverName;
    std::string installPath;
    std::string loadingMethod;
    std::string initiatedBy;
    unsigned long processId;
    std::string signerInfo;
    std::string timestamp;
    EventType eventType;
    ThreatLevel threatLevel;
    
    DriverEvent() : processId(0), eventType(EventType::Unsigned), threatLevel(ThreatLevel::Medium) {}
};

// Configuration structure
struct MonitorConfig {
    // Monitoring settings
    bool ignoreWindowsSigned;
    bool ignoreMicrosoft;
    bool blockUnsigned;
    bool verboseMode;
    
    // Alert settings
    bool playSound;
    bool showNotifications;
    
    // UI settings
    bool autoScroll;
    int maxEvents;
    
    // Logging settings
    bool loggingEnabled;
    std::string logFile;
    int maxLogSize;
    
    // Whitelist
    std::vector<std::string> whitelist;
    
    MonitorConfig() 
        : ignoreWindowsSigned(true)
        , ignoreMicrosoft(true)
        , blockUnsigned(false)
        , verboseMode(false)
        , playSound(true)
        , showNotifications(true)
        , autoScroll(true)
        , maxEvents(1000)
        , loggingEnabled(true)
        , logFile("driver_monitor.log")
        , maxLogSize(10485760)
    {}
};

// Utility functions
class Utils {
public:
    // Get current timestamp string
    static std::string GetTimestamp();
    
    // Get process name by PID
    static std::string GetProcessName(unsigned long pid);
    
    // Get signer information for a file
    static std::string GetSignerInfo(const std::string& filePath);
    
    // Check if file is signed by Microsoft
    static bool IsMicrosoftSigned(const std::string& signerInfo);
    
    // Check if file is Windows signed
    static bool IsWindowsSigned(const std::string& signerInfo);
    
    // Assess threat level based on driver info
    static ThreatLevel AssessThreatLevel(const DriverEvent& event);
    
    // Determine event type based on signer info
    static EventType DetermineEventType(const std::string& signerInfo, const std::string& loadingMethod);
    
    // Convert string to lowercase
    static std::string ToLower(const std::string& str);
    
    // Check if string contains substring (case-insensitive)
    static bool ContainsIgnoreCase(const std::string& haystack, const std::string& needle);
    
    // Format uptime as HH:MM:SS
    static std::string FormatUptime(int seconds);
};

} // namespace DriverMonitor
