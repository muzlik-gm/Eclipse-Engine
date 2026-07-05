// ============================================================================
// File: Engine/Include/Engine/Components/RigidBody.h
// Runtime representation of a rigid body.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;

    /// @brief Determines how a rigid body interacts with the physics world.
    enum class BodyType : int
    {
        Static    = 0,
        Dynamic   = 1,
        Kinematic = 2
    };

    /// @brief Attaches rigid body parameters to an entity.
    struct RigidBody
    {
        BodyType Type{BodyType::Dynamic};
        f32 Mass{1.0f};
        f32 LinearDrag{0.0f};
        f32 AngularDrag{0.05f};
        bool UseGravity{true};
        bool IsKinematic{false};
        float LinearVelocityX{0.0f};
        float LinearVelocityY{0.0f};
        float LinearVelocityZ{0.0f};
        float AngularVelocityX{0.0f};
        float AngularVelocityY{0.0f};
        float AngularVelocityZ{0.0f};
    };

} // namespace engine::components
