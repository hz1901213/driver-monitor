#pragma once

#include "../core/Utils.h"
#include <Windows.h>
#include <set>
#include <string>

namespace DriverMonitor {

class RegistryMonitor {
public:
    RegistryMonitor();
    ~RegistryMonitor();
    
    // Check for new drivers in registry
    DriverEvent CheckForNewDrivers();
    
private:
    std::set<std::string> m_knownDrivers;
    bool m_initialized;
    
    void Initialize();
    void ScanRegistry();
};

} // namespace DriverMonitor
