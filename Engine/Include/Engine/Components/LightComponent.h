// ============================================================================
// File: Engine/Include/Engine/Components/LightComponent.h
// Runtime light parameters.  Data only, no lighting implementation.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;
    using engine::core::u32;
    using engine::core::i32;
    using engine::math::Vec3;

    /// @brief Type of light source.
    enum class LightType : u32
    {
        None        = 0,
        Directional = 1,
        Point       = 2,
        Spot        = 3
    };

    /// @brief Attaches light parameters to an entity.
    struct LightComponent
    {
        LightType Type{LightType::None};
        Vec3 Color{1.0f, 1.0f, 1.0f};
        f32  Intensity{1.0f};
        f32  Range{10.0f};
        f32  SpotInnerAngle{12.5f};
        f32  SpotOuterAngle{17.5f};
        bool CastShadows{false};
        i32  ShadowMapResolution{1024};
        f32  ShadowBias{0.005f};
    };

} // namespace engine::components
