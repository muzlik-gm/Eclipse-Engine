// ============================================================================
// File: Engine/Include/Engine/Components/RigidBody.h
// Runtime representation of a rigid body.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"

namespace engine::components {

    using engine::core::f32;
    using engine::math::Vec3;

    /// @brief Determines how a rigid body interacts with the physics world.
    enum class BodyType : int
    {
        Static    = 0,
        Dynamic   = 1,
        Kinematic = 2
    };

    /// @brief Attaches rigid body parameters to an entity.
    ///
    /// Linear and angular velocities are stored as Vec3 for cache-friendly
    /// SIMD-friendly access by the future physics subsystem.
    struct RigidBody
    {
        BodyType Type{BodyType::Dynamic};
        f32 Mass{1.0f};
        f32 LinearDrag{0.0f};
        f32 AngularDrag{0.05f};
        bool UseGravity{true};
        bool IsKinematic{false};
        Vec3 LinearVelocity{0.0f, 0.0f, 0.0f};
        Vec3 AngularVelocity{0.0f, 0.0f, 0.0f};
    };

} // namespace engine::components
