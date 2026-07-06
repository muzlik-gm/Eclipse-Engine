// ============================================================================
// File: Engine/Include/Engine/Components/CameraComponent.h
// Runtime camera parameters for view/projection matrix generation.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;
    using engine::core::u32;
    using engine::math::Mat4;

    /// @brief Projection mode for the camera.
    enum class ProjectionMode : u32
    {
        Perspective = 0,
        Orthographic = 1
    };

    /// @brief Attaches camera parameters to an entity.
    struct CameraComponent
    {
        ProjectionMode Projection{ProjectionMode::Perspective};
        f32 FieldOfView{60.0f};
        f32 NearClip{0.1f};
        f32 FarClip{1000.0f};
        f32 OrthoSize{10.0f};
        bool Primary{false};
        f32 AspectRatio{0.0f};

        [[nodiscard]] Mat4 GetPerspectiveMatrix(f32 viewportAspect) const
        {
            f32 aspect = (AspectRatio > 0.0f) ? AspectRatio : viewportAspect;
            return math::Perspective(glm::radians(FieldOfView), aspect, NearClip, FarClip);
        }

        [[nodiscard]] Mat4 GetOrthographicMatrix(f32 viewportAspect) const
        {
            f32 aspect = (AspectRatio > 0.0f) ? AspectRatio : viewportAspect;
            f32 halfH = OrthoSize;
            f32 halfW = halfH * aspect;
            return math::Orthographic(-halfW, halfW, -halfH, halfH, NearClip, FarClip);
        }

        [[nodiscard]] Mat4 GetProjectionMatrix(f32 viewportAspect) const
        {
            return (Projection == ProjectionMode::Perspective)
                ? GetPerspectiveMatrix(viewportAspect)
                : GetOrthographicMatrix(viewportAspect);
        }
    };

} // namespace engine::components
