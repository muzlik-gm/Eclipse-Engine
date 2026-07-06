// ============================================================================
// File: Engine/Include/Engine/Renderer/Core/RendererStatistics.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include <string>
#include <vector>

namespace engine::renderer {

    using engine::core::u32;
    using engine::core::u64;
    using engine::core::f64;
    using engine::core::f32;

    /// @brief Per-frame rendering statistics.  Updated by the Renderer
    ///        after every frame and queryable by the editor / profiler.
    struct RendererStatistics
    {
        u64  FrameCount{0};
        f64  FrameTimeMs{0.0};
        f64  FrameTimeMin{0.0};
        f64  FrameTimeMax{0.0};
        f64  FrameTimeAvg{0.0};

        u32  DrawCalls{0};
        u32  DrawIndexedCalls{0};
        u32  DispatchCalls{0};
        u32  TrianglesDrawn{0};
        u32  VerticesDrawn{0};

        u32  BindPipelineCalls{0};
        u32  BindVertexBufferCalls{0};
        u32  BindIndexBufferCalls{0};
        u32  BindTextureCalls{0};
        u32  BindSamplerCalls{0};
        u32  BindUniformBufferCalls{0};
        u32  BindStorageBufferCalls{0};
        u32  BindFramebufferCalls{0};

        u32  SetViewportCalls{0};
        u32  SetScissorCalls{0};
        u32  ClearCalls{0};

        u32  TextureUploads{0};
        u32  BufferUploads{0};
        u64  TextureUploadBytes{0};
        u64  BufferUploadBytes{0};

        u32  ActiveTextures{0};
        u32  ActiveFramebuffers{0};
        u32  ActivePipelines{0};
        u32  ActiveShaders{0};

        f64  GPUTimeMs{0.0};

        void Reset();
        void UpdateAverage();
        [[nodiscard]] f32 GetFPS() const noexcept;
        [[nodiscard]] std::string ToString() const;
    };

} // namespace engine::renderer
