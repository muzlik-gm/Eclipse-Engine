// ============================================================================
// File: Engine/Source/Renderer/Core/RendererStatistics.cpp
// ============================================================================
#include "Engine/Renderer/Core/RendererStatistics.h"

namespace engine::renderer {

    void RendererStatistics::Reset()
    {
        FrameTimeMs = 0.0;
        DrawCalls = 0;
        DrawIndexedCalls = 0;
        DispatchCalls = 0;
        TrianglesDrawn = 0;
        VerticesDrawn = 0;
        BindPipelineCalls = 0;
        BindVertexBufferCalls = 0;
        BindIndexBufferCalls = 0;
        BindTextureCalls = 0;
        BindSamplerCalls = 0;
        BindUniformBufferCalls = 0;
        BindStorageBufferCalls = 0;
        BindFramebufferCalls = 0;
        SetViewportCalls = 0;
        SetScissorCalls = 0;
        ClearCalls = 0;
        TextureUploads = 0;
        BufferUploads = 0;
        TextureUploadBytes = 0;
        BufferUploadBytes = 0;
        GPUTimeMs = 0.0;
    }

    void RendererStatistics::UpdateAverage()
    {
        if (FrameTimeMs < FrameTimeMin || FrameCount == 1)
            FrameTimeMin = FrameTimeMs;
        if (FrameTimeMs > FrameTimeMax)
            FrameTimeMax = FrameTimeMs;

        // Exponential moving average.
        if (FrameCount > 1)
            FrameTimeAvg = FrameTimeAvg * 0.95 + FrameTimeMs * 0.05;
        else
            FrameTimeAvg = FrameTimeMs;
    }

    f32 RendererStatistics::GetFPS() const noexcept
    {
        if (FrameTimeAvg <= 0.0)
            return 0.0f;
        return static_cast<f32>(1000.0 / FrameTimeAvg);
    }

    std::string RendererStatistics::ToString() const
    {
        return std::string("Frame: ") + std::to_string(FrameCount)
             + " | FPS: " + std::to_string(GetFPS())
             + " | FrameTime: " + std::to_string(FrameTimeAvg) + "ms"
             + " | Draws: " + std::to_string(DrawCalls + DrawIndexedCalls)
             + " | Tris: " + std::to_string(TrianglesDrawn);
    }

} // namespace engine::renderer
