/**
 * @file PerformanceCounters.cpp
 * @brief Thread-safe named performance counter implementation.
 */

#include "Engine/Diagnostics/PerformanceCounters.h"

#include <algorithm>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace engine::diagnostics
{

// ===========================================================================
// Static storage
// ===========================================================================

std::vector<PerformanceCounter> PerformanceCounters::s_counters;
std::mutex                      PerformanceCounters::s_mutex;

// ===========================================================================
// PerformanceCounters
// ===========================================================================

PerformanceCounters::CounterId PerformanceCounters::RegisterCounter(std::string_view name)
{
    std::lock_guard lock(s_mutex);

    // Return existing id if a counter with this name is already registered.
    for (usize i = 0; i < s_counters.size(); ++i)
    {
        if (s_counters[i].name == name)
        {
            return static_cast<CounterId>(i);
        }
    }

    // Append a new counter.
    PerformanceCounter counter;
    counter.name     = name;
    counter.value    = 0;
    counter.hitCount = 0;
    s_counters.push_back(std::move(counter));
    return static_cast<CounterId>(s_counters.size() - 1);
}

void PerformanceCounters::Increment(CounterId id, u64 amount)
{
    std::lock_guard lock(s_mutex);

    if (id < s_counters.size())
    {
        s_counters[id].value += amount;
        ++s_counters[id].hitCount;
    }
}

void PerformanceCounters::Set(CounterId id, u64 value)
{
    std::lock_guard lock(s_mutex);

    if (id < s_counters.size())
    {
        s_counters[id].value = value;
    }
}

u64 PerformanceCounters::GetValue(CounterId id)
{
    std::lock_guard lock(s_mutex);

    if (id < s_counters.size())
    {
        return s_counters[id].value;
    }
    return 0;
}

u64 PerformanceCounters::GetHitCount(CounterId id)
{
    std::lock_guard lock(s_mutex);

    if (id < s_counters.size())
    {
        return s_counters[id].hitCount;
    }
    return 0;
}

void PerformanceCounters::Reset(CounterId id)
{
    std::lock_guard lock(s_mutex);

    if (id < s_counters.size())
    {
        s_counters[id].value    = 0;
        s_counters[id].hitCount = 0;
    }
}

void PerformanceCounters::ResetAll()
{
    std::lock_guard lock(s_mutex);

    for (auto& counter : s_counters)
    {
        counter.value    = 0;
        counter.hitCount = 0;
    }
}

std::vector<PerformanceCounter> PerformanceCounters::GetAllCounters()
{
    std::lock_guard lock(s_mutex);
    return s_counters;
}

std::string PerformanceCounters::Report()
{
    std::vector<PerformanceCounter> snapshot;
    {
        std::lock_guard lock(s_mutex);
        snapshot = s_counters;
    }

    if (snapshot.empty())
    {
        return "No performance counters registered.";
    }

    // Determine the longest counter name for column alignment.
    usize maxNameLen = 0;
    for (const auto& counter : snapshot)
    {
        maxNameLen = std::max(maxNameLen, counter.name.size());
    }

    std::ostringstream oss;
    oss << "=== Performance Counters ===\n";

    for (const auto& counter : snapshot)
    {
        oss << "  " << std::left << std::setw(static_cast<int>(maxNameLen))
            << counter.name << "  value: " << counter.value
            << "  hits: " << counter.hitCount << "\n";
    }

    return oss.str();
}

} // namespace engine::diagnostics