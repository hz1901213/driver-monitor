#pragma once

#include "../core/Utils.h"

namespace DriverMonitor {

class ETWConsumer {
public:
    ETWConsumer();
    ~ETWConsumer();
    
    // Start ETW session
    bool Start();
    
    // Stop ETW session
    void Stop();
    
    // Check for new driver events
    DriverEvent CheckForNewDrivers();
    
private:
    bool m_isRunning;
};

} // namespace DriverMonitor
