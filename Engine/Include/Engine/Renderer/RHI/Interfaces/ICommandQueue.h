// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/ICommandQueue.h
// Abstract interfaces for vertex arrays, command buffers, command queues,
// and the swapchain.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Interfaces/IBuffer.h"
#include "Engine/Renderer/RHI/Interfaces/ITexture.h"
#include "Engine/Renderer/RHI/Interfaces/IPipeline.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <span>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;
    using engine::core::usize;

    // ========================================================================
    // IVertexArray
    // ========================================================================

    /// @brief Vertex Array Object — encapsulates the binding of vertex
    ///        buffers and the vertex layout.  On OpenGL this maps directly
    ///        to a VAO; on other backends it is emulated.
    class IVertexArray : public IGraphicsObject
    {
    public:
        /// @brief Binds a vertex buffer to a binding index.
        virtual void BindVertexBuffer(u32 binding, IVertexBuffer* buffer, usize offset = 0) = 0;

        /// @brief Binds an index buffer.
        virtual void BindIndexBuffer(IIndexBuffer* buffer, IndexType type, usize offset = 0) = 0;

        /// @brief Sets the vertex layout for a binding index.
        virtual void SetVertexLayout(u32 binding, const VertexLayoutDescription& layout) = 0;

        /// @brief Returns the currently bound index type.
        [[nodiscard]] virtual IndexType GetIndexType() const noexcept = 0;

        /// @brief Returns the number of bound vertex buffers.
        [[nodiscard]] virtual u32 GetBoundVertexBufferCount() const noexcept = 0;
    };

    // ========================================================================
    // ICommandBuffer — records GPU commands for later submission.
    // ========================================================================

    /// @brief Records a sequence of GPU commands for later submission to a
    ///        command queue.  Command buffers are the only way to issue
    ///        draw, dispatch, copy, and barrier operations through the RHI.
    class ICommandBuffer : public IGraphicsObject
    {
    public:
        // -- Recording lifecycle -------------------------------------------

        /// @brief Begins recording.  Resets any previously recorded commands.
        virtual void Begin() = 0;

        /// @brief Ends recording.  The buffer cannot be modified after this.
        virtual void End() = 0;

        /// @brief Resets the buffer to its initial state.
        virtual void Reset() = 0;

        /// @brief Returns true if the buffer is currently recording.
        [[nodiscard]] virtual bool IsRecording() const noexcept = 0;

        /// @brief Returns true if the buffer has finished recording and is
        ///        ready for submission.
        [[nodiscard]] virtual bool IsExecutable() const noexcept = 0;

        // -- Frame / pass lifecycle ----------------------------------------

        virtual void BeginFrame() = 0;
        virtual void EndFrame()   = 0;
        virtual void Present()     = 0;

        virtual void BeginRenderPass(IRenderPass* renderPass, IFramebuffer* framebuffer) = 0;
        virtual void EndRenderPass() = 0;

        // -- Binding -------------------------------------------------------

        virtual void BindPipeline(IGraphicsPipeline* pipeline) = 0;
        virtual void BindComputePipeline(IComputePipeline* pipeline) = 0;
        virtual void BindVertexArray(IVertexArray* vao) = 0;
        virtual void BindVertexBuffer(u32 binding, IVertexBuffer* buffer, usize offset = 0) = 0;
        virtual void BindIndexBuffer(IIndexBuffer* buffer, usize offset = 0) = 0;
        virtual void BindTexture(u32 slot, ITexture* texture) = 0;
        virtual void BindSampler(u32 slot, ISampler* sampler) = 0;
        virtual void BindUniformBuffer(u32 slot, IUniformBuffer* buffer) = 0;
        virtual void BindStorageBuffer(u32 slot, IStorageBuffer* buffer) = 0;
        virtual void BindFramebuffer(IFramebuffer* framebuffer) = 0;

        // -- Push constants ------------------------------------------------

        virtual void PushConstants(ShaderStage stages, u32 offset, u32 size, const void* data) = 0;

        // -- State setting -------------------------------------------------

        virtual void SetViewport(const ViewportDescription& viewport) = 0;
        virtual void SetScissor(const ScissorDescription& scissor) = 0;
        virtual void SetViewportDynamic(f32 x, f32 y, f32 w, f32 h, f32 minDepth, f32 maxDepth) = 0;
        virtual void SetScissorDynamic(i32 x, i32 y, u32 w, u32 h) = 0;

        // -- Clearing ------------------------------------------------------

        virtual void ClearColor(u32 attachmentIndex, f32 r, f32 g, f32 b, f32 a) = 0;
        virtual void ClearDepth(f32 depth) = 0;
        virtual void ClearStencil(u32 stencil) = 0;
        virtual void ClearDepthStencil(f32 depth, u32 stencil) = 0;

        // -- Draws / dispatches -------------------------------------------

        virtual void Draw(u32 vertexCount, u32 instanceCount = 1,
                          u32 firstVertex = 0, u32 firstInstance = 0) = 0;

        virtual void DrawIndexed(u32 indexCount, u32 instanceCount = 1,
                                 u32 firstIndex = 0, i32 vertexOffset = 0,
                                 u32 firstInstance = 0) = 0;

        virtual void DrawInstanced(u32 vertexCountPerInstance, u32 instanceCount,
                                   u32 firstVertex, u32 firstInstance) = 0;

        virtual void DrawIndexedIndirect(IStorageBuffer* indirectBuffer,
                                         usize offset, u32 drawCount,
                                         usize stride) = 0;

        virtual void Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) = 0;

        // -- Copies / barriers --------------------------------------------

        virtual void CopyBuffer(IBuffer* src, IBuffer* dst,
                                usize srcOffset, usize dstOffset, usize size) = 0;

        virtual void CopyTexture(ITexture* src, ITexture* dst,
                                 u32 srcMip, u32 srcLayer,
                                 u32 dstMip, u32 dstLayer) = 0;

        virtual void CopyBufferToTexture(IBuffer* src, ITexture* dst,
                                         usize srcOffset,
                                         u32 dstMip, u32 dstLayer) = 0;

        virtual void CopyTextureToBuffer(ITexture* src, IBuffer* dst,
                                         u32 srcMip, u32 srcLayer,
                                         usize dstOffset) = 0;

        virtual void GenerateMipmaps(ITexture* texture) = 0;

        virtual void TransitionResource(IGraphicsObject* resource,
                                        ResourceState oldState,
                                        ResourceState newState) = 0;

        // -- Synchronization ----------------------------------------------

        virtual void Flush() = 0;
        virtual void Finish() = 0;

        /// @brief Inserts a debug group with the given name.
        virtual void BeginDebugGroup(std::string_view name) = 0;

        /// @brief Ends the most recently begun debug group.
        virtual void EndDebugGroup() = 0;

        /// @brief Inserts a single debug marker.
        virtual void InsertDebugMarker(std::string_view name) = 0;
    };

    // ========================================================================
    // ICommandQueue — submits command buffers to a hardware queue.
    // ========================================================================

    /// @brief Submits command buffers to a GPU hardware queue.  Each queue
    ///        has a type (Graphics / Compute / Transfer) and may support
    ///        presentation to a swapchain.
    class ICommandQueue : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual QueueType GetType() const noexcept = 0;

        /// @brief Submits one or more command buffers for execution.
        virtual void Submit(std::span<ICommandBuffer* const> commandBuffers) = 0;

        /// @brief Submits a single command buffer for execution.
        void Submit(ICommandBuffer* commandBuffer)
        {
            Submit(std::span<ICommandBuffer* const>(&commandBuffer, 1));
        }

        /// @brief Blocks until all submitted work has finished.
        virtual void WaitIdle() = 0;

        /// @brief Presents the swapchain image associated with this queue.
        virtual void Present(class ISwapChain* swapchain) = 0;
    };

    // ========================================================================
    // ISwapChain
    // ========================================================================

    /// @brief A swapchain — the chain of images presented to the screen.
    ///        Created from a SwapChainDescription and a window handle.
    class ISwapChain : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual const SwapChainDescription& GetDescription() const noexcept = 0;

        /// @brief Returns the current back-buffer texture.
        [[nodiscard]] virtual ITexture* GetCurrentBuffer() const noexcept = 0;

        /// @brief Returns the back-buffer texture at @p index.
        [[nodiscard]] virtual ITexture* GetBuffer(u32 index) const noexcept = 0;

        /// @brief Returns the number of back-buffer textures.
        [[nodiscard]] virtual u32 GetBufferCount() const noexcept = 0;

        /// @brief Acquires the next back-buffer image.  Called before
        ///        recording the frame's command buffer.
        virtual void AcquireNextImage() = 0;

        /// @brief Presents the current back-buffer image to the screen.
        virtual void Present() = 0;

        /// @brief Resizes the swapchain.  Must be called when the window
        ///        is resized.
        virtual void Resize(u32 width, u32 height) = 0;

        /// @brief Returns true if the swapchain needs to be resized
        ///        (e.g. window was resized).
        [[nodiscard]] virtual bool NeedsResize() const noexcept = 0;

        /// @brief Returns the current back-buffer index.
        [[nodiscard]] virtual u32 GetCurrentBufferIndex() const noexcept = 0;
    };

} // namespace engine::rhi
