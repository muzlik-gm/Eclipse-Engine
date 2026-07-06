// ============================================================================
// File: Engine/Include/Engine/Components/TransformComponent.h
// Position, rotation, and scale — the core spatial component.
// ============================================================================

#pragma once

#include "Engine/Math/Math.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine::components
{

    using engine::core::f32;

    // ========================================================================
    // TransformComponent
    // ========================================================================

    /// Stores translation, rotation (as a quaternion), and scale for an
    /// entity, and provides convenience methods to compute the combined
    /// local-space transformation matrix and convert between quaternion
    /// and Euler angle representations.
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

        // ----------------------------------------------------------------
        // Matrix generation
        // ----------------------------------------------------------------

        /// Computes the local-space TRS matrix (translate * rotate * scale).
        [[nodiscard]] math::Mat4 GetLocalMatrix() const
        {
            math::Mat4 transform = math::Mat4(1.0f);
            transform = glm::translate(transform, Translation);
            transform = transform * glm::mat4_cast(Rotation);
            transform = glm::scale(transform, Scale);
            return transform;
        }

        // ----------------------------------------------------------------
        // Euler angle support
        // ----------------------------------------------------------------

        /// @brief Returns the rotation as Euler angles in degrees.
        ///
        /// Euler angles are returned in the order (Pitch, Yaw, Roll)
        /// corresponding to rotation around (X, Y, Z) axes.  This is the
        /// most common convention for editor-facing UI.
        [[nodiscard]] math::Vec3 GetEulerDegrees() const
        {
            const math::Vec3 radians = glm::eulerAngles(Rotation);
            return math::Vec3(
                math::Degrees(radians.x),
                math::Degrees(radians.y),
                math::Degrees(radians.z)
            );
        }

        /// @brief Returns the rotation as Euler angles in radians.
        [[nodiscard]] math::Vec3 GetEulerRadians() const
        {
            return glm::eulerAngles(Rotation);
        }

        /// @brief Sets the rotation from Euler angles in degrees.
        ///
        /// @param pitchDeg   Rotation around X axis (degrees).
        /// @param yawDeg     Rotation around Y axis (degrees).
        /// @param rollDeg    Rotation around Z axis (degrees).
        void SetEulerDegrees(f32 pitchDeg, f32 yawDeg, f32 rollDeg)
        {
            Rotation = glm::quat(math::Vec3(
                math::Radians(pitchDeg),
                math::Radians(yawDeg),
                math::Radians(rollDeg)
            ));
            WorldDirty = true;
        }

        /// @brief Sets the rotation from Euler angles in radians.
        void SetEulerRadians(f32 pitchRad, f32 yawRad, f32 rollRad)
        {
            Rotation = glm::quat(math::Vec3(pitchRad, yawRad, rollRad));
            WorldDirty = true;
        }

        /// @brief Sets the rotation from a Vec3 of Euler angles in degrees.
        void SetEulerDegrees(const math::Vec3& eulerAnglesDeg)
        {
            SetEulerDegrees(eulerAnglesDeg.x, eulerAnglesDeg.y, eulerAnglesDeg.z);
        }

        // ----------------------------------------------------------------
        // Convenience setters
        // ----------------------------------------------------------------

        /// @brief Sets the translation and marks the world matrix dirty.
        void SetTranslation(const math::Vec3& translation)
        {
            Translation = translation;
            WorldDirty = true;
        }

        /// @brief Sets the rotation and marks the world matrix dirty.
        void SetRotation(const math::Quat& rotation)
        {
            Rotation = rotation;
            WorldDirty = true;
        }

        /// @brief Sets the scale and marks the world matrix dirty.
        void SetScale(const math::Vec3& scale)
        {
            Scale = scale;
            WorldDirty = true;
        }

        /// @brief Marks the world matrix as dirty so the TransformSystem
        ///        recomputes it on the next update.
        void SetDirty() { WorldDirty = true; }

        // ----------------------------------------------------------------
        // Cached state
        // ----------------------------------------------------------------

        /// Cached world-space transformation matrix (computed by the
        /// hierarchy system).
        math::Mat4 WorldMatrix{1.0f};

        /// True when the world matrix needs to be recomputed.
        bool WorldDirty{true};
    };

} // namespace engine::components
