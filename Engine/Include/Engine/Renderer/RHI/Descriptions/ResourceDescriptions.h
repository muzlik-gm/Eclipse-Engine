// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h
// Immutable description structs for GPU resources.  Every description is a
// value type with no virtual methods and no backend-specific members.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;
    using engine::core::usize;

    // ========================================================================
    // BufferDescription
    // ========================================================================

    /// @brief Immutable description used to create any GPU buffer.
    struct BufferDescription
    {
        /// Human-readable label for debugging and GPU object naming.
        std::string Name;

        /// Size in bytes of the buffer's storage.
        usize Size{0};

        /// How the buffer will be used by the GPU.  See BufferUsage.
        BufferUsage Usage{BufferUsage::None};

        /// Element format of the index buffer, if Usage includes IndexBuffer.
        IndexType IndexFormat{IndexType::UInt32};

        /// Stride between elements for vertex buffers (0 = unstructured).
        usize Stride{0};

        /// Initial resource state for barrier-aware backends.
        ResourceState InitialState{ResourceState::General};

        /// True if the buffer should be host-visible (CPU accessible).
        bool HostVisible{false};

        /// True if the buffer's contents will never change after creation.
        bool Immutable{false};

        /// Optional debug name forwarded to the backend's object labeling API.
        bool DebugNamed{true};

        [[nodiscard]] bool operator==(const BufferDescription& other) const noexcept
        {
            return Size == other.Size
                && Usage == other.Usage
                && IndexFormat == other.IndexFormat
                && Stride == other.Stride;
        }
    };

    // ========================================================================
    // TextureSubresourceRange
    // ========================================================================

    /// @brief Identifies a contiguous range of mips and array layers in a
    ///        texture.  Used by barriers, clears, and copy operations.
    struct TextureSubresourceRange
    {
        u32 BaseMipLevel{0};
        u32 MipLevelCount{1};
        u32 BaseArrayLayer{0};
        u32 ArrayLayerCount{1};
    };

    // ========================================================================
    // TextureDescription
    // ========================================================================

    /// @brief Immutable description used to create any GPU texture.
    struct TextureDescription
    {
        std::string     Name;

        TextureType     Type{TextureType::Texture2D};
        TextureFormat   Format{GraphicsFormat::RGBA8_UNorm};
        TextureUsage    Usage{TextureUsage::Sampled};

        u32 Width{1};
        u32 Height{1};
        u32 Depth{1};

        /// Number of mipmap levels (1 = no mipmaps).
        u32 MipLevels{1};

        /// Number of array layers (6 for cubemaps).
        u32 ArrayLayers{1};

        /// Number of samples per pixel (1 = no MSAA).
        u32 Samples{1};

        ResourceState InitialState{ResourceState::Sampled};

        bool DebugNamed{true};

        [[nodiscard]] bool IsCubemap() const noexcept
        {
            return (Usage & TextureUsage::CubeMap) != 0
                || Type == TextureType::TextureCube
                || Type == TextureType::TextureCubeArray;
        }

        [[nodiscard]] bool IsArray() const noexcept
        {
            return Type == TextureType::Texture1DArray
                || Type == TextureType::Texture2DArray
                || Type == TextureType::TextureCubeArray;
        }
    };

    // ========================================================================
    // SamplerDescription
    // ========================================================================

    /// @brief Immutable description used to create a texture sampler.
    struct SamplerDescription
    {
        std::string Name;

        FilterMode MinFilter{FilterMode::Linear};
        FilterMode MagFilter{FilterMode::Linear};
        FilterMode MipFilter{FilterMode::Linear};

        AddressMode AddressU{AddressMode::Repeat};
        AddressMode AddressV{AddressMode::Repeat};
        AddressMode AddressW{AddressMode::Repeat};

        /// Border color used when AddressMode::ClampToBorder is active.
        std::array<f32, 4> BorderColor{0.0f, 0.0f, 0.0f, 0.0f};

        /// Maximum anisotropy (1.0 = no anisotropic filtering).
        f32 MaxAnisotropy{1.0f};

        /// Enable anisotropic filtering.
        bool AnisotropyEnable{false};

        /// Compare operation for shadow / depth-compare samplers.
        CompareOperation CompareOp{CompareOperation::Never};
        bool CompareEnable{false};

        /// LOD bias and clamps.
        f32 MipLodBias{0.0f};
        f32 MinLod{-1000.0f};
        f32 MaxLod{1000.0f};

        bool DebugNamed{true};
    };

    // ========================================================================
    // AttachmentDescription
    // ========================================================================

    /// @brief Describes a single attachment of a render pass / framebuffer.
    struct AttachmentDescription
    {
        GraphicsFormat           Format{GraphicsFormat::RGBA8_UNorm};
        u32                      Samples{1};
        AttachmentLoadOperation  LoadOp{AttachmentLoadOperation::Clear};
        AttachmentStoreOperation StoreOp{AttachmentStoreOperation::Store};
        AttachmentLoadOperation  StencilLoadOp{AttachmentLoadOperation::DontCare};
        AttachmentStoreOperation StencilStoreOp{AttachmentStoreOperation::DontCare};
        ResourceState            InitialState{ResourceState::ColorAttachment};
        ResourceState            FinalState{ResourceState::ColorAttachment};

        /// Clear color used when LoadOp == Clear.
        std::array<f32, 4> ClearColor{0.0f, 0.0f, 0.0f, 1.0f};

        /// Clear depth value used when LoadOp == Clear on a depth attachment.
        f32 ClearDepth{1.0f};

        /// Clear stencil value used when LoadOp == Clear on a stencil attachment.
        u32 ClearStencil{0};
    };

    // ========================================================================
    // RenderPassDescription
    // ========================================================================

    /// @brief Immutable description of a render pass — the set of
    ///        attachments, subpasses, and dependencies between them.
    struct RenderPassDescription
    {
        std::string Name;

        std::vector<AttachmentDescription> ColorAttachments;
        AttachmentDescription              DepthStencilAttachment;
        bool                               HasDepthStencil{false};

        u32 Width{0};
        u32 Height{0};

        bool DebugNamed{true};
    };

    // ========================================================================
    // FramebufferDescription
    // ========================================================================

    /// @brief Immutable description of a framebuffer — the concrete
    ///        texture views bound to a render pass.
    struct FramebufferDescription
    {
        std::string Name;

        u32 Width{0};
        u32 Height{0};
        u32 Layers{1};

        /// Pointer-stable references to color attachment textures.
        /// The backend interprets these as opaque ITexture* values.
        std::vector<const void*> ColorAttachments;

        /// Optional depth/stencil attachment.
        const void* DepthStencilAttachment{nullptr};

        bool DebugNamed{true};
    };

} // namespace engine::rhi
