#include "RegistryMonitor.h"
#include <vector>

namespace DriverMonitor {

RegistryMonitor::RegistryMonitor() : m_initialized(false) {
}

RegistryMonitor::~RegistryMonitor() {
}

void RegistryMonitor::Initialize() {
    if (m_initialized) {
        return;
    }
    
    // Scan current drivers to populate known list
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        char subKeyName[256];
        DWORD subKeyNameSize = sizeof(subKeyName);
        
        while (RegEnumKeyExA(hKey, index++, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            // Check if it's a driver service
            HKEY hSubKey;
            std::string subKeyPath = "SYSTEM\\CurrentControlSet\\Services\\";
            subKeyPath += subKeyName;
            
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                DWORD serviceType = 0;
                DWORD dataSize = sizeof(DWORD);
                
                if (RegQueryValueExA(hSubKey, "Type", nullptr, nullptr, (LPBYTE)&serviceType, &dataSize) == ERROR_SUCCESS) {
                    // SERVICE_KERNEL_DRIVER = 0x00000001
                    // SERVICE_FILE_SYSTEM_DRIVER = 0x00000002
                    if (serviceType == 1 || serviceType == 2) {
                        m_knownDrivers.insert(std::string(subKeyName));
                    }
                }
                
                RegCloseKey(hSubKey);
            }
            
            subKeyNameSize = sizeof(subKeyName);
        }
        
        RegCloseKey(hKey);
    }
    
    m_initialized = true;
}

DriverEvent RegistryMonitor::CheckForNewDrivers() {
    if (!m_initialized) {
        Initialize();
    }
    
    DriverEvent event;
    
    // Check for new drivers
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        char subKeyName[256];
        DWORD subKeyNameSize = sizeof(subKeyName);
        
        while (RegEnumKeyExA(hKey, index++, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            // Check if it's a new driver
            if (m_knownDrivers.find(std::string(subKeyName)) == m_knownDrivers.end()) {
                // Check if it's a driver service
                HKEY hSubKey;
                std::string subKeyPath = "SYSTEM\\CurrentControlSet\\Services\\";
                subKeyPath += subKeyName;
                
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    DWORD serviceType = 0;
                    DWORD dataSize = sizeof(DWORD);
                    
                    if (RegQueryValueExA(hSubKey, "Type", nullptr, nullptr, (LPBYTE)&serviceType, &dataSize) == ERROR_SUCCESS) {
                        if (serviceType == 1 || serviceType == 2) {
                            // Found a new driver!
                            m_knownDrivers.insert(std::string(subKeyName));
                            
                            event.driverName = std::string(subKeyName);
                            event.loadingMethod = "Registry - Service Installation";
                            
                            // Try to get image path
                            char imagePath[MAX_PATH] = {0};
                            DWORD imagePathSize = sizeof(imagePath);
                            if (RegQueryValueExA(hSubKey, "ImagePath", nullptr, nullptr, (LPBYTE)imagePath, &imagePathSize) == ERROR_SUCCESS) {
                                event.installPath = std::string(imagePath);
                            }
                            
                            // Get process info (usually services.exe)
                            event.initiatedBy = "services.exe";
                            event.processId = 0; // Unknown
                            
                            RegCloseKey(hSubKey);
                            RegCloseKey(hKey);
                            return event;
                        }
                    }
                    
                    RegCloseKey(hSubKey);
                }
            }
            
            subKeyNameSize = sizeof(subKeyName);
        }
        
        RegCloseKey(hKey);
    }
    
    return event;
}

} // namespace DriverMonitor
