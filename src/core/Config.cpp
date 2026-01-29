#include "Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

// Simple JSON parser (minimal implementation for our needs)
namespace {
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
    
    std::string unquote(const std::string& str) {
        std::string s = trim(str);
        if (s.length() >= 2 && s[0] == '"' && s[s.length()-1] == '"') {
            return s.substr(1, s.length()-2);
        }
        return s;
    }
    
    bool parseBool(const std::string& str) {
        std::string s = trim(str);
        return s == "true";
    }
    
    int parseInt(const std::string& str) {
        try {
            return std::stoi(trim(str));
        } catch (...) {
            return 0;
        }
    }
}

namespace DriverMonitor {

Config::Config() {
    m_configPath = "config.json";
}

Config::~Config() {
}

bool Config::Load(const std::string& filePath) {
    m_configPath = filePath;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // File doesn't exist, use defaults
        return false;
    }
    
    // Simple JSON parsing (basic implementation)
    std::string line;
    std::string section;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/' || line == "{" || line == "}") {
            continue;
        }
        
        // Check for section headers
        if (line.find("\"monitoring\"") != std::string::npos) {
            section = "monitoring";
            continue;
        } else if (line.find("\"alerts\"") != std::string::npos) {
            section = "alerts";
            continue;
        } else if (line.find("\"ui\"") != std::string::npos) {
            section = "ui";
            continue;
        } else if (line.find("\"logging\"") != std::string::npos) {
            section = "logging";
            continue;
        } else if (line.find("\"whitelist\"") != std::string::npos) {
            section = "whitelist";
            continue;
        }
        
        // Parse key-value pairs
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Remove quotes and trailing comma
            key = unquote(key);
            if (value.back() == ',') {
                value = value.substr(0, value.length() - 1);
            }
            value = trim(value);
            
            // Parse based on section
            if (section == "monitoring") {
                if (key == "ignoreWindowsSigned") m_config.ignoreWindowsSigned = parseBool(value);
                else if (key == "ignoreMicrosoft") m_config.ignoreMicrosoft = parseBool(value);
                else if (key == "blockUnsigned") m_config.blockUnsigned = parseBool(value);
                else if (key == "verboseMode") m_config.verboseMode = parseBool(value);
            } else if (section == "alerts") {
                if (key == "playSound") m_config.playSound = parseBool(value);
                else if (key == "showNotifications") m_config.showNotifications = parseBool(value);
            } else if (section == "ui") {
                if (key == "autoScroll") m_config.autoScroll = parseBool(value);
                else if (key == "maxEvents") m_config.maxEvents = parseInt(value);
            } else if (section == "logging") {
                if (key == "enabled") m_config.loggingEnabled = parseBool(value);
                else if (key == "logFile") m_config.logFile = unquote(value);
                else if (key == "maxLogSize") m_config.maxLogSize = parseInt(value);
            }
        }
        
        // Parse whitelist array items
        if (section == "whitelist" && line.find('"') != std::string::npos) {
            std::string item = unquote(line);
            if (!item.empty() && item != "[" && item != "]") {
                m_config.whitelist.push_back(item);
            }
        }
    }
    
    file.close();
    return true;
}

bool Config::Save(const std::string& filePath) {
    std::ofstream file(filePath.empty() ? m_configPath : filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"monitoring\": {\n";
    file << "    \"ignoreWindowsSigned\": " << (m_config.ignoreWindowsSigned ? "true" : "false") << ",\n";
    file << "    \"ignoreMicrosoft\": " << (m_config.ignoreMicrosoft ? "true" : "false") << ",\n";
    file << "    \"blockUnsigned\": " << (m_config.blockUnsigned ? "true" : "false") << ",\n";
    file << "    \"verboseMode\": " << (m_config.verboseMode ? "true" : "false") << "\n";
    file << "  },\n";
    file << "  \"alerts\": {\n";
    file << "    \"playSound\": " << (m_config.playSound ? "true" : "false") << ",\n";
    file << "    \"showNotifications\": " << (m_config.showNotifications ? "true" : "false") << "\n";
    file << "  },\n";
    file << "  \"ui\": {\n";
    file << "    \"autoScroll\": " << (m_config.autoScroll ? "true" : "false") << ",\n";
    file << "    \"maxEvents\": " << m_config.maxEvents << "\n";
    file << "  },\n";
    file << "  \"logging\": {\n";
    file << "    \"enabled\": " << (m_config.loggingEnabled ? "true" : "false") << ",\n";
    file << "    \"logFile\": \"" << m_config.logFile << "\",\n";
    file << "    \"maxLogSize\": " << m_config.maxLogSize << "\n";
    file << "  },\n";
    file << "  \"whitelist\": [\n";
    
    for (size_t i = 0; i < m_config.whitelist.size(); ++i) {
        file << "    \"" << m_config.whitelist[i] << "\"";
        if (i < m_config.whitelist.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

void Config::AddToWhitelist(const std::string& driverName) {
    if (!IsWhitelisted(driverName)) {
        m_config.whitelist.push_back(driverName);
    }
}

void Config::RemoveFromWhitelist(const std::string& driverName) {
    auto it = std::find(m_config.whitelist.begin(), m_config.whitelist.end(), driverName);
    if (it != m_config.whitelist.end()) {
        m_config.whitelist.erase(it);
    }
}

bool Config::IsWhitelisted(const std::string& driverName) const {
    return std::find(m_config.whitelist.begin(), m_config.whitelist.end(), driverName) != m_config.whitelist.end();
}

} // namespace DriverMonitor
