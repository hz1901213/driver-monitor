#include "EventManager.h"
#include <algorithm>

namespace DriverMonitor {

EventManager::EventManager() 
    : m_lastReadIndex(0)
    , m_maxEvents(1000)
    , m_signedCount(0)
    , m_unsignedCount(0)
    , m_suspiciousCount(0) {
}

EventManager::~EventManager() {
}

void EventManager::AddEvent(const DriverEvent& event) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Add event
    m_events.push_back(event);
    
    // Update statistics
    switch (event.eventType) {
        case EventType::Signed:
            m_signedCount++;
            break;
        case EventType::Unsigned:
            m_unsignedCount++;
            break;
        case EventType::Suspicious:
            m_suspiciousCount++;
            break;
    }
    
    // Enforce max events limit
    if (m_events.size() > static_cast<size_t>(m_maxEvents)) {
        // Remove oldest event
        EventType removedType = m_events.front().eventType;
        m_events.erase(m_events.begin());
        
        // Update statistics
        switch (removedType) {
            case EventType::Signed:
                if (m_signedCount > 0) m_signedCount--;
                break;
            case EventType::Unsigned:
                if (m_unsignedCount > 0) m_unsignedCount--;
                break;
            case EventType::Suspicious:
                if (m_suspiciousCount > 0) m_suspiciousCount--;
                break;
        }
        
        // Adjust read index if necessary
        if (m_lastReadIndex > 0) {
            m_lastReadIndex--;
        }
    }
}

std::vector<DriverEvent> EventManager::GetEvents() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_events;
}

std::vector<DriverEvent> EventManager::GetNewEvents() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<DriverEvent> newEvents;
    
    if (m_lastReadIndex < m_events.size()) {
        newEvents.insert(newEvents.end(), 
                        m_events.begin() + m_lastReadIndex, 
                        m_events.end());
        m_lastReadIndex = m_events.size();
    }
    
    return newEvents;
}

void EventManager::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.clear();
    m_lastReadIndex = 0;
    m_signedCount = 0;
    m_unsignedCount = 0;
    m_suspiciousCount = 0;
}

size_t EventManager::GetEventCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_events.size();
}

int EventManager::GetSignedCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_signedCount;
}

int EventManager::GetUnsignedCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_unsignedCount;
}

int EventManager::GetSuspiciousCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_suspiciousCount;
}

void EventManager::SetMaxEvents(int maxEvents) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxEvents = maxEvents;
    
    // Trim events if necessary
    while (m_events.size() > static_cast<size_t>(m_maxEvents)) {
        EventType removedType = m_events.front().eventType;
        m_events.erase(m_events.begin());
        
        switch (removedType) {
            case EventType::Signed:
                if (m_signedCount > 0) m_signedCount--;
                break;
            case EventType::Unsigned:
                if (m_unsignedCount > 0) m_unsignedCount--;
                break;
            case EventType::Suspicious:
                if (m_suspiciousCount > 0) m_suspiciousCount--;
                break;
        }
        
        if (m_lastReadIndex > 0) {
            m_lastReadIndex--;
        }
    }
}

} // namespace DriverMonitor
