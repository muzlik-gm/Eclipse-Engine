// ============================================================================
// File: Engine/Include/Engine/Math/Math.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine::math {

    using engine::core::f32;
    using engine::core::f64;
    using engine::core::i32;
    using engine::core::u32;

    // ========================================================================
    // Type aliases – common GLM types
    // ========================================================================

    // -- 2-component vectors --------------------------------------------------
    using Vec2  = glm::vec<2, f32>;
    using Vec2d = glm::vec<2, f64>;
    using IVec2 = glm::vec<2, i32>;
    using UVec2 = glm::vec<2, u32>;

    // -- 3-component vectors --------------------------------------------------
    using Vec3  = glm::vec<3, f32>;
    using Vec3d = glm::vec<3, f64>;
    using IVec3 = glm::vec<3, i32>;
    using UVec3 = glm::vec<3, u32>;

    // -- 4-component vectors --------------------------------------------------
    using Vec4  = glm::vec<4, f32>;
    using Vec4d = glm::vec<4, f64>;
    using IVec4 = glm::vec<4, i32>;
    using UVec4 = glm::vec<4, u32>;

    // -- Matrices -------------------------------------------------------------
    using Mat3 = glm::mat<3, 3, f32>;
    using Mat4 = glm::mat<4, 4, f32>;

    // -- Quaternion -----------------------------------------------------------
    using Quat = glm::qua<f32>;

    // ========================================================================
    // Common constants
    // ========================================================================

    inline constexpr Vec3 kZeroVec3{0.0f, 0.0f, 0.0f};
    inline constexpr Vec3 kOneVec3 {1.0f, 1.0f, 1.0f};
    inline constexpr Vec3 kUp      {0.0f, 1.0f, 0.0f};
    inline constexpr Vec3 kDown    {0.0f,-1.0f, 0.0f};
    inline constexpr Vec3 kForward {0.0f, 0.0f, 1.0f};
    inline constexpr Vec3 kBack    {0.0f, 0.0f,-1.0f};
    inline constexpr Vec3 kRight   {1.0f, 0.0f, 0.0f};
    inline constexpr Vec3 kLeft    {-1.0f, 0.0f, 0.0f};

    // ========================================================================
    // Utility functions
    // ========================================================================

    /// Build a perspective projection matrix.
    [[nodiscard]] inline Mat4 Perspective(f32 fovY, f32 aspect, f32 zNear, f32 zFar)
    {
        return glm::perspective(fovY, aspect, zNear, zFar);
    }

    /// Build an orthographic projection matrix.
    [[nodiscard]] inline Mat4 Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
    {
        return glm::ortho(left, right, bottom, top, zNear, zFar);
    }

    /// Build a look-at view matrix.
    [[nodiscard]] inline Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
    {
        return glm::lookAt(eye, center, up);
    }

    /// Convert a Quat to a Mat4.
    [[nodiscard]] inline Mat4 ToMat4(const Quat& q)
    {
        return glm::mat4_cast(q);
    }

    // -- Vector operations (scalar Vec3 overloads for convenience) ------------

    [[nodiscard]] inline f32 Normalize(f32 value) { return glm::normalize(Vec3{value, 0.0f, 0.0f}).x; }
    [[nodiscard]] inline Vec3 Normalize(const Vec3& v) { return glm::normalize(v); }

    [[nodiscard]] inline f32 Dot(const Vec3& a, const Vec3& b) { return glm::dot(a, b); }
    [[nodiscard]] inline Vec3 Cross(const Vec3& a, const Vec3& b) { return glm::cross(a, b); }

    [[nodiscard]] inline f32 Length(const Vec3& v) { return glm::length(v); }
    [[nodiscard]] inline f32 LengthSquared(const Vec3& v) { return glm::length2(v); }

    /// Linear interpolation between two values.
    template <typename T>
    [[nodiscard]] inline T Lerp(const T& a, const T& b, f32 t)
    {
        return glm::mix(a, b, t);
    }

    /// Convert radians to degrees.
    [[nodiscard]] inline f32 Degrees(f32 radians) { return glm::degrees(radians); }

    /// Convert degrees to radians.
    [[nodiscard]] inline f32 Radians(f32 degrees) { return glm::radians(degrees); }

} // namespace engine::math