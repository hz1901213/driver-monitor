#pragma once

#include "../core/Utils.h"
#include <Windows.h>
#include <set>
#include <string>

namespace DriverMonitor {

class FileSystemMonitor {
public:
    FileSystemMonitor();
    ~FileSystemMonitor();
    
    // Check for new drivers in file system
    DriverEvent CheckForNewDrivers();
    
private:
    std::set<std::string> m_knownDriverFiles;
    bool m_initialized;
    std::string m_driversPath;
    
    void Initialize();
    void ScanDirectory();
};

} // namespace DriverMonitor
