// ============================================================================
// File: Engine/Include/Engine/Transforms/TransformUtils.h
// Free functions for composing and decomposing transformation matrices.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine::transforms {

// ========================================================================
// ComposeTRS
// ========================================================================

/// @brief Builds a TRS matrix from individual translation, rotation,
///        and scale vectors.
[[nodiscard]] inline math::Mat4 ComposeTRS(
    const math::Vec3& pos, const math::Quat& rot, const math::Vec3& scale)
{
    math::Mat4 transform = math::Mat4(1.0f);
    transform = glm::translate(transform, pos);
    transform = transform * glm::mat4_cast(rot);
    transform = glm::scale(transform, scale);
    return transform;
}

// ========================================================================
// LocalToWorld
// ========================================================================

/// @brief Concatenates a parent world matrix with a local transform.
[[nodiscard]] inline math::Mat4 LocalToWorld(
    const math::Mat4& parentWorld, const math::Mat4& localTransform)
{
    return parentWorld * localTransform;
}

// ========================================================================
// Decompose helpers
// ========================================================================

/// @brief Extracts the translation column from a transformation matrix.
[[nodiscard]] inline math::Vec3 DecomposePosition(const math::Mat4& m)
{
    return math::Vec3(m[3][0], m[3][1], m[3][2]);
}

/// @brief Extracts the rotation as a quaternion from a
///        transformation matrix using GLM's quat_cast.
[[nodiscard]] inline math::Quat DecomposeRotation(const math::Mat4& m)
{
    return glm::quat_cast(m);
}

/// @brief Extracts the scale vector from a transformation matrix
///        by measuring the length of each basis column.
[[nodiscard]] inline math::Vec3 DecomposeScale(const math::Mat4& m)
{
    math::Vec3 result(
        glm::length(math::Vec3(m[0])),
        glm::length(math::Vec3(m[1])),
        glm::length(math::Vec3(m[2]))
    );
    return result;
}

} // namespace engine::transforms