// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLCommandBuffer.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"
#include "OpenGLStateCache.h"

#include <glad/glad.h>
#include <string>
#include <vector>

namespace engine::opengl {

    class OpenGLDebugLayer;
    class OpenGLGraphicsPipeline;
    class OpenGLFramebuffer;
    class OpenGLRenderPass;
    class OpenGLVertexArray;
    class OpenGLVertexBuffer;
    class OpenGLIndexBuffer;
    class OpenGLUniformBuffer;
    class OpenGLStorageBuffer;
    class OpenGLTexture;
    class OpenGLSampler;

    class OpenGLCommandBuffer final : public engine::rhi::ICommandBuffer
    {
    public:
        OpenGLCommandBuffer(OpenGLDebugLayer* debugLayer);
        ~OpenGLCommandBuffer() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override { m_Name = std::string(name); }
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        // -- Recording lifecycle
        void Begin() override;
        void End() override;
        void Reset() override;
        bool IsRecording() const noexcept override { return m_Recording; }
        bool IsExecutable() const noexcept override { return m_Executable; }

        // -- Frame / pass lifecycle
        void BeginFrame() override;
        void EndFrame() override;
        void Present() override;
        void BeginRenderPass(engine::rhi::IRenderPass* renderPass,
                             engine::rhi::IFramebuffer* framebuffer) override;
        void EndRenderPass() override;

        // -- Binding
        void BindPipeline(engine::rhi::IGraphicsPipeline* pipeline) override;
        void BindComputePipeline(engine::rhi::IComputePipeline* pipeline) override;
        void BindVertexArray(engine::rhi::IVertexArray* vao) override;
        void BindVertexBuffer(engine::core::u32 binding,
                              engine::rhi::IVertexBuffer* buffer,
                              engine::core::usize offset = 0) override;
        void BindIndexBuffer(engine::rhi::IIndexBuffer* buffer,
                             engine::core::usize offset = 0) override;
        void BindTexture(engine::core::u32 slot, engine::rhi::ITexture* texture) override;
        void BindSampler(engine::core::u32 slot, engine::rhi::ISampler* sampler) override;
        void BindUniformBuffer(engine::core::u32 slot, engine::rhi::IUniformBuffer* buffer) override;
        void BindStorageBuffer(engine::core::u32 slot, engine::rhi::IStorageBuffer* buffer) override;
        void BindFramebuffer(engine::rhi::IFramebuffer* framebuffer) override;

        // -- Push constants
        void PushConstants(engine::rhi::ShaderStage stages,
                           engine::core::u32 offset,
                           engine::core::u32 size,
                           const void* data) override;

        // -- State setting
        void SetViewport(const engine::rhi::ViewportDescription& viewport) override;
        void SetScissor(const engine::rhi::ScissorDescription& scissor) override;
        void SetViewportDynamic(engine::core::f32 x, engine::core::f32 y,
                                engine::core::f32 w, engine::core::f32 h,
                                engine::core::f32 minDepth,
                                engine::core::f32 maxDepth) override;
        void SetScissorDynamic(engine::core::i32 x, engine::core::i32 y,
                               engine::core::u32 w, engine::core::u32 h) override;

        // -- Clearing
        void ClearColor(engine::core::u32 attachmentIndex,
                        engine::core::f32 r, engine::core::f32 g,
                        engine::core::f32 b, engine::core::f32 a) override;
        void ClearDepth(engine::core::f32 depth) override;
        void ClearStencil(engine::core::u32 stencil) override;
        void ClearDepthStencil(engine::core::f32 depth, engine::core::u32 stencil) override;

        // -- Draws / dispatches
        void Draw(engine::core::u32 vertexCount, engine::core::u32 instanceCount = 1,
                  engine::core::u32 firstVertex = 0,
                  engine::core::u32 firstInstance = 0) override;
        void DrawIndexed(engine::core::u32 indexCount, engine::core::u32 instanceCount = 1,
                         engine::core::u32 firstIndex = 0,
                         engine::core::i32 vertexOffset = 0,
                         engine::core::u32 firstInstance = 0) override;
        void DrawInstanced(engine::core::u32 vertexCountPerInstance,
                           engine::core::u32 instanceCount,
                           engine::core::u32 firstVertex,
                           engine::core::u32 firstInstance) override;
        void DrawIndexedIndirect(engine::rhi::IStorageBuffer* indirectBuffer,
                                 engine::core::usize offset,
                                 engine::core::u32 drawCount,
                                 engine::core::usize stride) override;
        void Dispatch(engine::core::u32 groupCountX,
                      engine::core::u32 groupCountY,
                      engine::core::u32 groupCountZ) override;

        // -- Copies / barriers
        void CopyBuffer(engine::rhi::IBuffer* src, engine::rhi::IBuffer* dst,
                        engine::core::usize srcOffset,
                        engine::core::usize dstOffset,
                        engine::core::usize size) override;
        void CopyTexture(engine::rhi::ITexture* src, engine::rhi::ITexture* dst,
                         engine::core::u32 srcMip, engine::core::u32 srcLayer,
                         engine::core::u32 dstMip, engine::core::u32 dstLayer) override;
        void CopyBufferToTexture(engine::rhi::IBuffer* src, engine::rhi::ITexture* dst,
                                 engine::core::usize srcOffset,
                                 engine::core::u32 dstMip,
                                 engine::core::u32 dstLayer) override;
        void CopyTextureToBuffer(engine::rhi::ITexture* src, engine::rhi::IBuffer* dst,
                                 engine::core::u32 srcMip, engine::core::u32 srcLayer,
                                 engine::core::usize dstOffset) override;
        void GenerateMipmaps(engine::rhi::ITexture* texture) override;
        void TransitionResource(engine::rhi::IGraphicsObject* resource,
                                engine::rhi::ResourceState oldState,
                                engine::rhi::ResourceState newState) override;

        // -- Synchronization
        void Flush() override;
        void Finish() override;

        // -- Debug
        void BeginDebugGroup(std::string_view name) override;
        void EndDebugGroup() override;
        void InsertDebugMarker(std::string_view name) override;

    private:
        OpenGLDebugLayer*                 m_DebugLayer{nullptr};
        OpenGLStateCache                  m_StateCache;
        std::string                       m_Name;

        bool                              m_Recording{false};
        bool                              m_Executable{false};
        bool                              m_InRenderPass{false};

        OpenGLGraphicsPipeline*           m_BoundPipeline{nullptr};
        OpenGLVertexArray*                m_BoundVAO{nullptr};
        OpenGLIndexBuffer*                m_BoundIndexBuffer{nullptr};
        engine::rhi::IndexType            m_IndexType{engine::rhi::IndexType::UInt32};
        OpenGLFramebuffer*                m_BoundFramebuffer{nullptr};
        OpenGLRenderPass*                 m_ActiveRenderPass{nullptr};
    };

} // namespace engine::opengl
