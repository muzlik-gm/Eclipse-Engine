// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/IGraphicsObject.h
// Base interface for every RHI object.  Provides a stable virtual
// destructor, type identification, debug naming, and a backend-agnostic
// handle accessor.  No backend symbols appear here.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <string>
#include <string_view>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::u64;

    // ========================================================================
    // IGraphicsObject — base interface for all GPU resources.
    // ========================================================================

    /// @brief Base interface for every graphics object created through the
    ///        RHI.  Provides:
    ///          - a virtual destructor so the backend can release GPU
    ///            memory through the correct derived type;
    ///          - debug naming so GPU object labels can be inspected in
    ///            graphics debuggers;
    ///          - a type discriminator so the validation layer can check
    ///            that the correct interface is being used.
    class IGraphicsObject
    {
    public:
        virtual ~IGraphicsObject() = default;

        /// @brief Returns the human-readable debug name of this object.
        [[nodiscard]] virtual std::string_view GetDebugName() const noexcept = 0;

        /// @brief Sets the debug name.  Backend implementations forward
        ///        this to the underlying GPU object labeling API.
        virtual void SetDebugName(std::string_view name) = 0;

        /// @brief Returns the raw native handle, cast to u64.
        ///        The interpretation is backend-specific and must not be
        ///        relied upon by code outside the backend module.
        [[nodiscard]] virtual u64 GetNativeHandle() const noexcept = 0;

        /// @brief Returns true if the underlying GPU resource is still
        ///        valid and has not been destroyed.
        [[nodiscard]] virtual bool IsValid() const noexcept = 0;
    };

    // ========================================================================
    // ObjectType — discriminator for runtime type checks.
    // ========================================================================
    enum class ObjectType : u32
    {
        Unknown = 0,
        Buffer,
        Texture,
        Sampler,
        Shader,
        ShaderModule,
        ShaderProgram,
        VertexArray,
        Pipeline,
        PipelineLayout,
        RenderPass,
        Framebuffer,
        RenderTarget,
        SwapChain,
        CommandQueue,
        CommandBuffer,
        DescriptorSet,
        DescriptorLayout,
        DescriptorPool,
        Fence,
        Semaphore,
        GraphicsDevice,
        GraphicsContext,
        GraphicsAdapter,
        GraphicsInstance,
        GraphicsFactory
    };

} // namespace engine::rhi
