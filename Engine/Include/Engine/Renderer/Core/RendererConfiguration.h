// ============================================================================
// File: Engine/Include/Engine/Renderer/Core/RendererConfiguration.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <string>

namespace engine::renderer {

    using engine::core::u32;
    using engine::core::f32;

    /// @brief Configuration for the Renderer.  Set before Initialize().
    struct RendererConfiguration
    {
        /// Graphics backend to use.
        engine::rhi::GraphicsBackend Backend{engine::rhi::GraphicsBackend::OpenGL};

        /// Window handle (platform-specific, passed to the backend).
        void* WindowHandle{nullptr};

        /// Initial framebuffer dimensions.
        u32   Width{1280};
        u32   Height{720};

        /// Swapchain buffer count.
        u32   BufferCount{2};

        /// Swapchain format.
        engine::rhi::GraphicsFormat SwapchainFormat{engine::rhi::GraphicsFormat::RGBA8_UNorm};

        /// Enable vsync.
        bool  VSync{true};

        /// Enable validation / debug layers.
        bool  EnableValidation{true};
        bool  EnableDebugOutput{true};

        /// Enable state caching (almost always true).
        bool  EnableStateCache{true};

        /// Maximum frames in flight (for double/triple buffering of
        /// per-frame command buffers).
        u32   FramesInFlight{2};

        /// Shader include search paths.
        std::vector<std::string> ShaderIncludePaths;

        /// Shader cache directory (empty = no caching).
        std::string ShaderCacheDir;

        /// Pipeline cache directory (empty = no caching).
        std::string PipelineCacheDir;

        /// Default clear color.
        f32   DefaultClearColor[4]{0.1f, 0.1f, 0.1f, 1.0f};

        /// Default clear depth.
        f32   DefaultClearDepth{1.0f};
    };

} // namespace engine::renderer
