// ============================================================================
// File: Engine/Include/Engine/Components/Collider.h
// Runtime representation of a collision shape.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;
    using engine::math::Vec3;

    /// @brief Shape of the collider.
    enum class ColliderType : int
    {
        None    = 0,
        Box     = 1,
        Sphere  = 2,
        Capsule = 3,
        Mesh    = 4
    };

    /// @brief Attaches collision shape parameters to an entity.
    struct Collider
    {
        ColliderType Type{ColliderType::None};
        Vec3 Size{1.0f, 1.0f, 1.0f};
        Vec3 Center{0.0f, 0.0f, 0.0f};
        bool IsTrigger{false};
        f32  Friction{0.5f};
        f32  Restitution{0.0f};
    };

} // namespace engine::components
