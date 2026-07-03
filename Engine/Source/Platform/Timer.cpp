// ============================================================================
// File: Engine/Source/Platform/Timer.cpp
// Implementation of ITimer static queries using std::chrono.
// ============================================================================

#include "Engine/Platform/Timer.h"

#include <chrono>

namespace engine::platform
{

    f64 ITimer::GetAbsoluteTime()
    {
        static constexpr f64 kNanosecondsPerSecond = 1e-9;
        const auto now = std::chrono::high_resolution_clock::now();
        const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
        return static_cast<f64>(nanos) * kNanosecondsPerSecond;
    }

    u64 ITimer::GetAbsoluteTimeNanoseconds()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        return static_cast<u64>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()).count());
    }

} // namespace engine::platform