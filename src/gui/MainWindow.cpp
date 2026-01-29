#include "MainWindow.h"
#include <imgui.h>
#include <fstream>
#include <algorithm>
#include <Windows.h>

namespace DriverMonitor {

MainWindow::MainWindow(EventManager* eventManager, Config* config, DriverMonitor* monitor)
    : m_eventManager(eventManager)
    , m_config(config)
    , m_monitor(monitor)
    , m_selectedEventIndex(-1)
    , m_filterType(0)
    , m_showDetailsPanel(false) {
    memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
}

MainWindow::~MainWindow() {
}

void MainWindow::Render() {
    // Main window with docking
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        ImGui::Text("Windows Driver Monitor v2.0");
        ImGui::EndMenuBar();
    }
    
    // DockSpace
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    
    ImGui::End();
    
    // Render all panels
    RenderControlPanel();
    RenderStatisticsPanel();
    RenderSettingsPanel();
    RenderFilterPanel();
    RenderEventLogPanel();
    
    if (m_showDetailsPanel && m_selectedEventIndex >= 0) {
        RenderDetailsPanel();
    }
}

void MainWindow::RenderControlPanel() {
    ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Controls")) {
        bool isMonitoring = m_monitor->IsMonitoring();
        
        // Status indicator
        ImGui::Text("Status:");
        ImGui::SameLine();
        if (isMonitoring) {
            ImGui::TextColored(ImVec4(0.188f, 0.788f, 0.690f, 1.0f), "ON");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "OFF");
        }
        
        ImGui::Separator();
        ImGui::Spacing();
        
        // Start/Stop button
        if (isMonitoring) {
            if (ImGui::Button("Stop Monitoring", ImVec2(-1, 0))) {
                m_monitor->Stop();
            }
        } else {
            if (ImGui::Button("Start Monitoring", ImVec2(-1, 0))) {
                m_monitor->Start();
            }
        }
        
        ImGui::Spacing();
        
        // Save configuration button
        if (ImGui::Button("Save Config", ImVec2(-1, 0))) {
            m_config->Save();
        }
        
        ImGui::Spacing();
        
        // Export logs button
        if (ImGui::Button("Export Logs", ImVec2(-1, 0))) {
            ExportLogs();
        }
    }
    ImGui::End();
}

void MainWindow::RenderStatisticsPanel() {
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Statistics")) {
        int signedCount = m_eventManager->GetSignedCount();
        int unsignedCount = m_eventManager->GetUnsignedCount();
        int suspiciousCount = m_eventManager->GetSuspiciousCount();
        int totalCount = signedCount + unsignedCount + suspiciousCount;
        
        ImGui::Text("Drivers Detected: %d", totalCount);
        ImGui::TextColored(ImVec4(0.188f, 0.788f, 0.690f, 1.0f), "Signed: %d", signedCount);
        ImGui::TextColored(ImVec4(0.808f, 0.569f, 0.471f, 1.0f), "Unsigned: %d", unsignedCount);
        ImGui::TextColored(ImVec4(0.957f, 0.529f, 0.443f, 1.0f), "Suspicious: %d", suspiciousCount);
        
        ImGui::Separator();
        
        if (m_monitor->IsMonitoring()) {
            int uptime = m_monitor->GetUptimeSeconds();
            std::string uptimeStr = Utils::FormatUptime(uptime);
            ImGui::Text("Uptime: %s", uptimeStr.c_str());
        } else {
            ImGui::TextDisabled("Not monitoring");
        }
    }
    ImGui::End();
}

void MainWindow::RenderSettingsPanel() {
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Settings")) {
        auto& config = m_config->GetConfig();
        
        ImGui::Text("MONITORING OPTIONS");
        ImGui::Separator();
        
        if (ImGui::Checkbox("Ignore Windows Signed Drivers", &config.ignoreWindowsSigned)) {
            // Configuration changed
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Filter out drivers signed by Microsoft Windows");
        }
        
        if (ImGui::Checkbox("Ignore Microsoft Drivers", &config.ignoreMicrosoft)) {
            // Configuration changed
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Filter out all Microsoft signed drivers");
        }
        
        ImGui::Checkbox("Block Unsigned Drivers", &config.blockUnsigned);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Attempt to block unsigned driver loading (requires admin)");
        }
        
        ImGui::Checkbox("Verbose Mode", &config.verboseMode);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Show all events including filtered ones");
        }
        
        ImGui::Spacing();
        ImGui::Text("ALERT OPTIONS");
        ImGui::Separator();
        
        ImGui::Checkbox("Play Alert Sound", &config.playSound);
        ImGui::Checkbox("Show Notifications", &config.showNotifications);
        
        ImGui::Spacing();
        ImGui::Text("UI OPTIONS");
        ImGui::Separator();
        
        ImGui::Checkbox("Auto-scroll Log", &config.autoScroll);
        
        int maxEvents = config.maxEvents;
        if (ImGui::SliderInt("Max Events", &maxEvents, 100, 5000)) {
            config.maxEvents = maxEvents;
            m_eventManager->SetMaxEvents(maxEvents);
        }
        
        ImGui::Spacing();
        ImGui::Text("LOGGING OPTIONS");
        ImGui::Separator();
        
        ImGui::Checkbox("Enable Logging", &config.loggingEnabled);
        
        char logFilePath[256];
        strncpy_s(logFilePath, config.logFile.c_str(), sizeof(logFilePath) - 1);
        if (ImGui::InputText("Log File", logFilePath, sizeof(logFilePath))) {
            config.logFile = std::string(logFilePath);
        }
        
        ImGui::Spacing();
        
        // Whitelist management
        ImGui::Text("WHITELIST");
        ImGui::Separator();
        
        if (ImGui::BeginChild("WhitelistChild", ImVec2(0, 100), true)) {
            auto& whitelist = config.whitelist;
            
            for (size_t i = 0; i < whitelist.size(); ++i) {
                ImGui::Text("%s", whitelist[i].c_str());
                ImGui::SameLine();
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::SmallButton("Remove")) {
                    m_config->RemoveFromWhitelist(whitelist[i]);
                }
                ImGui::PopID();
            }
            
            if (whitelist.empty()) {
                ImGui::TextDisabled("No whitelisted drivers");
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void MainWindow::RenderFilterPanel() {
    ImGui::SetNextWindowSize(ImVec2(600, 100), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Filters")) {
        ImGui::Text("Show:");
        ImGui::SameLine();
        
        const char* filterItems[] = { "All", "Signed", "Unsigned", "Suspicious" };
        ImGui::SetNextItemWidth(150);
        ImGui::Combo("##FilterType", &m_filterType, filterItems, IM_ARRAYSIZE(filterItems));
        
        ImGui::SameLine();
        ImGui::Text("Search:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##Search", m_searchBuffer, sizeof(m_searchBuffer));
        
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        }
    }
    ImGui::End();
}

void MainWindow::RenderEventLogPanel() {
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Event Log")) {
        // Clear button
        if (ImGui::Button("Clear Log")) {
            m_eventManager->Clear();
            m_selectedEventIndex = -1;
            m_showDetailsPanel = false;
        }
        
        ImGui::Separator();
        
        // Get filtered events
        auto filteredEvents = GetFilteredEvents();
        
        // Event log table
        if (ImGui::BeginTable("EventTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Driver", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Method", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Signer", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(filteredEvents.size()));
            
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                    const DriverEvent* event = filteredEvents[row];
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    
                    // Make row selectable
                    ImGui::PushID(row);
                    bool isSelected = (row == m_selectedEventIndex);
                    
                    if (ImGui::Selectable("##row", isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                        m_selectedEventIndex = row;
                        m_showDetailsPanel = true;
                    }
                    
                    // Context menu
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Copy Details")) {
                            std::string details = event->timestamp + " " + event->driverName + " - " + event->signerInfo;
                            if (OpenClipboard(nullptr)) {
                                EmptyClipboard();
                                HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, details.size() + 1);
                                if (hg) {
                                    memcpy(GlobalLock(hg), details.c_str(), details.size() + 1);
                                    GlobalUnlock(hg);
                                    SetClipboardData(CF_TEXT, hg);
                                }
                                CloseClipboard();
                            }
                        }
                        if (ImGui::MenuItem("Add to Whitelist")) {
                            m_config->AddToWhitelist(event->driverName);
                        }
                        ImGui::EndPopup();
                    }
                    
                    // Time
                    ImGui::Text("%s", event->timestamp.c_str());
                    
                    // Status icon and text
                    ImGui::TableNextColumn();
                    ImVec4 color = GetEventColor(event->eventType);
                    ImGui::TextColored(color, "%s", GetEventIcon(event->eventType));
                    
                    // Driver name
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", event->driverName.c_str());
                    
                    // Loading method
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", event->loadingMethod.c_str());
                    
                    // Signer
                    ImGui::TableNextColumn();
                    ImGui::TextColored(color, "%s", event->signerInfo.c_str());
                    
                    ImGui::PopID();
                }
            }
            
            // Auto-scroll
            if (m_config->GetConfig().autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            
            ImGui::EndTable();
        }
        
        ImGui::Text("Total Events: %zu", filteredEvents.size());
    }
    ImGui::End();
}

void MainWindow::RenderDetailsPanel() {
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Event Details", &m_showDetailsPanel)) {
        auto allEvents = m_eventManager->GetEvents();
        
        if (m_selectedEventIndex >= 0 && m_selectedEventIndex < static_cast<int>(allEvents.size())) {
            const auto& event = allEvents[m_selectedEventIndex];
            
            ImGui::Text("Driver Name:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s", event.driverName.c_str());
            
            ImGui::Separator();
            
            ImGui::Text("Path: %s", event.installPath.c_str());
            ImGui::Text("Method: %s", event.loadingMethod.c_str());
            ImGui::Text("Initiated By: %s (PID: %lu)", event.initiatedBy.c_str(), event.processId);
            
            ImGui::Separator();
            
            ImGui::Text("Signer:");
            ImGui::SameLine();
            ImVec4 color = GetEventColor(event.eventType);
            ImGui::TextColored(color, "%s", event.signerInfo.c_str());
            
            ImGui::Text("Threat Level:");
            ImGui::SameLine();
            const char* threatStr = GetThreatLevelString(event.threatLevel);
            ImVec4 threatColor;
            switch (event.threatLevel) {
                case ThreatLevel::Low: threatColor = ImVec4(0.188f, 0.788f, 0.690f, 1.0f); break;
                case ThreatLevel::Medium: threatColor = ImVec4(0.808f, 0.569f, 0.471f, 1.0f); break;
                case ThreatLevel::High: threatColor = ImVec4(0.957f, 0.529f, 0.443f, 1.0f); break;
            }
            ImGui::TextColored(threatColor, "%s", threatStr);
            
            ImGui::Separator();
            ImGui::Spacing();
            
            // Action buttons
            if (ImGui::Button("Copy Details")) {
                std::string details = "Driver: " + event.driverName + "\n" +
                                    "Path: " + event.installPath + "\n" +
                                    "Method: " + event.loadingMethod + "\n" +
                                    "Initiated By: " + event.initiatedBy + "\n" +
                                    "Signer: " + event.signerInfo + "\n" +
                                    "Threat: " + std::string(threatStr);
                
                if (OpenClipboard(nullptr)) {
                    EmptyClipboard();
                    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, details.size() + 1);
                    if (hg) {
                        memcpy(GlobalLock(hg), details.c_str(), details.size() + 1);
                        GlobalUnlock(hg);
                        SetClipboardData(CF_TEXT, hg);
                    }
                    CloseClipboard();
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Add to Whitelist")) {
                m_config->AddToWhitelist(event.driverName);
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("View File Location")) {
                if (!event.installPath.empty()) {
                    std::string command = "explorer /select," + event.installPath;
                    system(command.c_str());
                }
            }
        }
    }
    ImGui::End();
}

std::vector<const DriverEvent*> MainWindow::GetFilteredEvents() const {
    std::vector<const DriverEvent*> filtered;
    auto allEvents = m_eventManager->GetEvents();
    
    for (const auto& event : allEvents) {
        // Apply search filter
        if (!MatchesSearch(event)) {
            continue;
        }
        
        // Apply type filter
        if (!MatchesTypeFilter(event)) {
            continue;
        }
        
        filtered.push_back(&event);
    }
    
    return filtered;
}

bool MainWindow::MatchesSearch(const DriverEvent& event) const {
    if (strlen(m_searchBuffer) == 0) {
        return true;
    }
    
    std::string search = Utils::ToLower(m_searchBuffer);
    std::string driverName = Utils::ToLower(event.driverName);
    std::string path = Utils::ToLower(event.installPath);
    
    return driverName.find(search) != std::string::npos ||
           path.find(search) != std::string::npos;
}

bool MainWindow::MatchesTypeFilter(const DriverEvent& event) const {
    if (m_filterType == 0) {
        return true; // All
    } else if (m_filterType == 1) {
        return event.eventType == EventType::Signed;
    } else if (m_filterType == 2) {
        return event.eventType == EventType::Unsigned;
    } else if (m_filterType == 3) {
        return event.eventType == EventType::Suspicious;
    }
    return true;
}

void MainWindow::ExportLogs() {
    auto events = m_eventManager->GetEvents();
    
    std::ofstream file("driver_monitor_export.txt");
    if (!file.is_open()) {
        return;
    }
    
    file << "Driver Monitor Event Log Export\n";
    file << "================================\n\n";
    
    for (const auto& event : events) {
        file << event.timestamp << " [" << GetEventIcon(event.eventType) << "] "
             << event.driverName << "\n";
        file << "  Path: " << event.installPath << "\n";
        file << "  Method: " << event.loadingMethod << "\n";
        file << "  Signer: " << event.signerInfo << "\n";
        file << "  Threat: " << GetThreatLevelString(event.threatLevel) << "\n\n";
    }
    
    file.close();
}

ImVec4 MainWindow::GetEventColor(EventType type) const {
    switch (type) {
        case EventType::Signed:
            return ImVec4(0.188f, 0.788f, 0.690f, 1.0f); // Teal
        case EventType::Unsigned:
            return ImVec4(0.808f, 0.569f, 0.471f, 1.0f); // Orange
        case EventType::Suspicious:
            return ImVec4(0.957f, 0.529f, 0.443f, 1.0f); // Red
    }
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

const char* MainWindow::GetEventIcon(EventType type) const {
    switch (type) {
        case EventType::Signed: return "OK";
        case EventType::Unsigned: return "WARN";
        case EventType::Suspicious: return "CRIT";
    }
    return "?";
}

const char* MainWindow::GetThreatLevelString(ThreatLevel level) const {
    switch (level) {
        case ThreatLevel::Low: return "Low";
        case ThreatLevel::Medium: return "Medium";
        case ThreatLevel::High: return "High";
    }
    return "Unknown";
}

} // namespace DriverMonitor
