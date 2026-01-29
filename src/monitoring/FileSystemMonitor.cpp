#include "FileSystemMonitor.h"

namespace DriverMonitor {

FileSystemMonitor::FileSystemMonitor() : m_initialized(false) {
    // Get Windows directory
    char winDir[MAX_PATH];
    GetWindowsDirectoryA(winDir, MAX_PATH);
    m_driversPath = std::string(winDir) + "\\System32\\drivers";
}

FileSystemMonitor::~FileSystemMonitor() {
}

void FileSystemMonitor::Initialize() {
    if (m_initialized) {
        return;
    }
    
    ScanDirectory();
    m_initialized = true;
}

void FileSystemMonitor::ScanDirectory() {
    WIN32_FIND_DATAA findData;
    std::string searchPath = m_driversPath + "\\*.sys";
    
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                m_knownDriverFiles.insert(std::string(findData.cFileName));
            }
        } while (FindNextFileA(hFind, &findData));
        
        FindClose(hFind);
    }
}

DriverEvent FileSystemMonitor::CheckForNewDrivers() {
    if (!m_initialized) {
        Initialize();
    }
    
    DriverEvent event;
    
    WIN32_FIND_DATAA findData;
    std::string searchPath = m_driversPath + "\\*.sys";
    
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string fileName(findData.cFileName);
                
                // Check if this is a new file
                if (m_knownDriverFiles.find(fileName) == m_knownDriverFiles.end()) {
                    m_knownDriverFiles.insert(fileName);
                    
                    event.driverName = fileName;
                    event.installPath = m_driversPath + "\\" + fileName;
                    event.loadingMethod = "File System - New Driver File";
                    event.initiatedBy = "Unknown";
                    event.processId = 0;
                    
                    FindClose(hFind);
                    return event;
                }
            }
        } while (FindNextFileA(hFind, &findData));
        
        FindClose(hFind);
    }
    
    return event;
}

} // namespace DriverMonitor
