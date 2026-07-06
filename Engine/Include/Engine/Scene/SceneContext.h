// ============================================================================
// File: Engine/Include/Engine/Scene/SceneContext.h
// Provides shared context that systems can access during scene updates.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"

#include <chrono>

namespace engine::scene {

    using engine::core::f64;
    using engine::core::u64;
    using engine::math::Vec2;

    /// @brief Immutable per-frame context passed to every system.
    class SceneContext
    {
    public:
        f64  DeltaTime{0.0};
        f64  FixedDeltaTime{1.0 / 60.0};
        f64  ElapsedTime{0.0};
        Vec2 ViewportSize{1280.0f, 720.0f};
        u64  FrameNumber{0};
        std::chrono::high_resolution_clock::time_point FrameStart;

        SceneContext() = default;
    };

} // namespace engine::scene
