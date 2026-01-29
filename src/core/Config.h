#pragma once

#include "Utils.h"
#include <string>

namespace DriverMonitor {

class Config {
public:
    Config();
    ~Config();
    
    // Load configuration from JSON file
    bool Load(const std::string& filePath = "config.json");
    
    // Save configuration to JSON file
    bool Save(const std::string& filePath = "config.json");
    
    // Get configuration
    MonitorConfig& GetConfig() { return m_config; }
    const MonitorConfig& GetConfig() const { return m_config; }
    
    // Add driver to whitelist
    void AddToWhitelist(const std::string& driverName);
    
    // Remove driver from whitelist
    void RemoveFromWhitelist(const std::string& driverName);
    
    // Check if driver is whitelisted
    bool IsWhitelisted(const std::string& driverName) const;
    
private:
    MonitorConfig m_config;
    std::string m_configPath;
};

} // namespace DriverMonitor
