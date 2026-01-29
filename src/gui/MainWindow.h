#pragma once

#include "../core/EventManager.h"
#include "../core/Config.h"
#include "../core/DriverMonitor.h"
#include <string>
#include <vector>

namespace DriverMonitor {

class MainWindow {
public:
    MainWindow(EventManager* eventManager, Config* config, DriverMonitor* monitor);
    ~MainWindow();
    
    // Render the main window
    void Render();
    
private:
    EventManager* m_eventManager;
    Config* m_config;
    DriverMonitor* m_monitor;
    
    // UI state
    int m_selectedEventIndex;
    char m_searchBuffer[256];
    int m_filterType; // 0=All, 1=Signed, 2=Unsigned, 3=Suspicious
    bool m_showDetailsPanel;
    
    // Render panels
    void RenderControlPanel();
    void RenderStatisticsPanel();
    void RenderSettingsPanel();
    void RenderFilterPanel();
    void RenderEventLogPanel();
    void RenderDetailsPanel();
    
    // Get filtered events
    std::vector<const DriverEvent*> GetFilteredEvents() const;
    
    // Apply search filter
    bool MatchesSearch(const DriverEvent& event) const;
    
    // Apply type filter
    bool MatchesTypeFilter(const DriverEvent& event) const;
    
    // Export logs to file
    void ExportLogs();
    
    // Get color for event type
    ImVec4 GetEventColor(EventType type) const;
    
    // Get icon for event type
    const char* GetEventIcon(EventType type) const;
    
    // Get threat level string
    const char* GetThreatLevelString(ThreatLevel level) const;
};

} // namespace DriverMonitor
