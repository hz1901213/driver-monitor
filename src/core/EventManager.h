#pragma once

#include "Utils.h"
#include <vector>
#include <mutex>
#include <queue>
#include <memory>

namespace DriverMonitor {

class EventManager {
public:
    EventManager();
    ~EventManager();
    
    // Add event to queue
    void AddEvent(const DriverEvent& event);
    
    // Get all events
    std::vector<DriverEvent> GetEvents() const;
    
    // Get events since last call (for incremental updates)
    std::vector<DriverEvent> GetNewEvents();
    
    // Clear all events
    void Clear();
    
    // Get event count
    size_t GetEventCount() const;
    
    // Get statistics
    int GetSignedCount() const;
    int GetUnsignedCount() const;
    int GetSuspiciousCount() const;
    
    // Set maximum event count
    void SetMaxEvents(int maxEvents);
    
private:
    mutable std::mutex m_mutex;
    std::vector<DriverEvent> m_events;
    size_t m_lastReadIndex;
    int m_maxEvents;
    
    // Statistics counters
    int m_signedCount;
    int m_unsignedCount;
    int m_suspiciousCount;
};

} // namespace DriverMonitor
