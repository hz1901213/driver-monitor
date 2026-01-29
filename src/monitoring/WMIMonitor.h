#pragma once

#include "../core/Utils.h"
#include <set>
#include <string>

namespace DriverMonitor {

class WMIMonitor {
public:
    WMIMonitor();
    ~WMIMonitor();
    
    // Check for new drivers via WMI
    DriverEvent CheckForNewDrivers();
    
private:
    std::set<std::string> m_knownDrivers;
    bool m_initialized;
    
    void Initialize();
};

} // namespace DriverMonitor
