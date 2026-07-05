// ============================================================================
// File: Engine/Include/Engine/Components/TransformComponent.h
// Position, rotation, and scale — the core spatial component.
// ============================================================================

#pragma once

#include "Engine/Math/Math.h"

#include <glm/gtc/matrix_transform.hpp>

namespace engine::components
{

    // ========================================================================
    // TransformComponent
    // ========================================================================

    /// Stores translation, rotation (as a quaternion), and scale for an
    /// entity, and provides a convenience method to compute the combined
    /// local-space transformation matrix.
    ///
    /// The world-space matrix is cached separately and marked dirty when
    /// any local property changes.
    struct TransformComponent
    {
        /// Local-space translation.
        math::Vec3 Translation{0.0f, 0.0f, 0.0f};

        /// Local-space rotation (identity quaternion by default).
        math::Quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};

        /// Local-space non-uniform scale.
        math::Vec3 Scale{1.0f, 1.0f, 1.0f};

        /// Computes the local-space TRS matrix (translate * rotate * scale).
        [[nodiscard]] math::Mat4 GetLocalMatrix() const
        {
            math::Mat4 transform = math::Mat4(1.0f);
            transform = glm::translate(transform, Translation);
            transform = transform * glm::mat4_cast(Rotation);
            transform = glm::scale(transform, Scale);
            return transform;
        }

        /// Cached world-space transformation matrix (computed by the
        /// hierarchy system).
        math::Mat4 WorldMatrix{1.0f};

        /// True when the world matrix needs to be recomputed.
        bool WorldDirty{true};
    };

} // namespace engine::components