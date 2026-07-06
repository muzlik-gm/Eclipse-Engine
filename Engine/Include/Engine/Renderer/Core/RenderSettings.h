// ============================================================================
// File: Engine/Include/Engine/Renderer/Core/RenderSettings.h
// Configurable renderer settings.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace engine::renderer {

    using engine::core::u32;
    using engine::core::f32;

    // ========================================================================
    // RenderSettings — all configurable renderer options.
    // ========================================================================

    struct RenderSettings
    {
        // -- Display -------------------------------------------------------
        bool  VSync{true};
        u32   MSAA{1};            // 1 = off, 2, 4, 8
        bool  HDR{false};
        f32   Gamma{2.2f};
        f32   Exposure{1.0f};
        f32   RenderScale{1.0f};  // 1.0 = native, 0.5 = half res

        // -- Quality -------------------------------------------------------
        u32   ShadowQuality{1};   // 0=off, 1=low, 2=med, 3=high
        u32   TextureFiltering{1}; // 0=nearest, 1=bilinear, 2=trilinear, 3=anisotropic
        u32   AnisotropyLevel{4};

        // -- Debug ---------------------------------------------------------
        bool  Wireframe{false};
        bool  ShowGrid{true};
        bool  ShowBoundingBoxes{false};
        bool  ShowNormals{false};
        bool  ShowFrustums{false};
        bool  ShowLightIcons{true};

        // -- Render modes --------------------------------------------------
        enum class DebugMode : u32 { None = 0, Albedo, Normals, Depth, Wireframe };
        DebugMode CurrentDebugMode{DebugMode::None};

        // -- Scene view ----------------------------------------------------
        f32   EditorCameraFOV{60.0f};
        f32   EditorCameraNear{0.1f};
        f32   EditorCameraFar{1000.0f};
        f32   EditorCameraSpeed{5.0f};

        // -- Limits --------------------------------------------------------
        u32   MaxDrawCalls{100000};
        u32   MaxVerticesPerFrame{10000000};
    };

} // namespace engine::renderer
