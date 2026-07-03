#pragma once

/**
 * @file PerformanceCounters.h
 * @brief Named performance counters with thread-safe access.
 *
 * Provides a global registry of named u64 counters that can be incremented,
 * set, and queried from any thread.  A convenience Report() method formats
 * all counters for logging or display.
 */

#include "Engine/Core/Types.h"

#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace engine::diagnostics
{

using engine::core::u64;
using engine::core::usize;

// ===========================================================================
// PerformanceCounter
// ===========================================================================

/// @brief A single named counter that tracks a cumulative value and a hit count.
struct PerformanceCounter
{
    std::string name;    ///< Human-readable counter name.
    u64         value    = 0;  ///< Current counter value.
    u64         hitCount = 0;  ///< Number of times the counter has been incremented.
};

// ===========================================================================
// PerformanceCounters
// ===========================================================================

/// @brief Global registry of named performance counters.
///
/// Thread-safe.  All methods are static; the class cannot be instantiated.
///
/// Typical usage:
/// @code
///   auto id = PerformanceCounters::RegisterCounter("DrawCalls");
///   PerformanceCounters::Increment(id);
///   ENGINE_INFO("{}", PerformanceCounters::Report());
/// @endcode
class PerformanceCounters
{
public:
    using CounterId = usize;

    PerformanceCounters() = delete;

    /// @brief Registers a new counter and returns its opaque handle.
    /// @return A CounterId that can be passed to Increment / Set / GetValue.
    ///         If a counter with the same name already exists its existing
    ///         id is returned instead.
    static CounterId RegisterCounter(std::string_view name);

    /// @brief Adds @p amount (default 1) to the counter's value and
    ///        increments its hit count.
    static void Increment(CounterId id, u64 amount = 1);

    /// @brief Sets the counter's value to @p value without changing the hit count.
    static void Set(CounterId id, u64 value);

    /// @brief Returns the current value of the counter, or 0 if @p id is invalid.
    [[nodiscard]] static u64 GetValue(CounterId id);

    /// @brief Returns the number of times Increment() has been called on this
    ///        counter, or 0 if @p id is invalid.
    [[nodiscard]] static u64 GetHitCount(CounterId id);

    /// @brief Resets the counter's value and hit count to zero.
    static void Reset(CounterId id);

    /// @brief Resets every registered counter to zero.
    static void ResetAll();

    /// @brief Returns a snapshot of all registered counters.
    [[nodiscard]] static std::vector<PerformanceCounter> GetAllCounters();

    /// @brief Returns a formatted multi-line string listing every counter,
    ///        its value, and its hit count.
    [[nodiscard]] static std::string Report();

private:
    static std::vector<PerformanceCounter> s_counters;
    static std::mutex                      s_mutex;
};

} // namespace engine::diagnostics