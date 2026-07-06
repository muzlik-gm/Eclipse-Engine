// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/ITexture.h
// Abstract interfaces for GPU textures and samplers.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <span>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::usize;

    // ========================================================================
    // TextureData — CPU-side source data for a texture upload.
    // ========================================================================

    /// @brief Describes a single mip level of CPU-side texture data.
    struct TextureData
    {
        const void* Pixels{nullptr};
        usize       Size{0};         // size in bytes
        u32         Width{0};
        u32         Height{0};
        u32         Depth{1};
    };

    // ========================================================================
    // ITexture — base interface for all GPU textures.
    // ========================================================================

    /// @brief Base interface for every GPU texture type.
    class ITexture : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual const TextureDescription& GetDescription() const noexcept = 0;

        [[nodiscard]] virtual TextureType     GetType() const noexcept = 0;
        [[nodiscard]] virtual TextureFormat   GetFormat() const noexcept = 0;
        [[nodiscard]] virtual TextureUsage    GetUsage() const noexcept = 0;

        [[nodiscard]] virtual u32 GetWidth()     const noexcept = 0;
        [[nodiscard]] virtual u32 GetHeight()    const noexcept = 0;
        [[nodiscard]] virtual u32 GetDepth()     const noexcept = 0;
        [[nodiscard]] virtual u32 GetMipLevels() const noexcept = 0;
        [[nodiscard]] virtual u32 GetArrayLayers() const noexcept = 0;
        [[nodiscard]] virtual u32 GetSamples()   const noexcept = 0;

        /// @brief Uploads pixel data into a specific mip / layer.
        ///        The backend handles format conversion and alignment.
        ///
        /// @param mipLevel     Destination mip level.
        /// @param arrayLayer   Destination array layer.
        /// @param data         Source pixel data.
        /// @param xOffset      Destination X offset inside the mip.
        /// @param yOffset      Destination Y offset inside the mip.
        /// @param width        Width of the upload region.
        /// @param height       Height of the upload region.
        virtual void UploadData(u32 mipLevel, u32 arrayLayer,
                                const TextureData& data,
                                u32 xOffset = 0, u32 yOffset = 0,
                                u32 width = 0, u32 height = 0) = 0;

        /// @brief Generates all remaining mip levels from level 0.
        virtual void GenerateMipmaps() = 0;

        /// @brief Transitions the texture to a new resource state.
        virtual void TransitionTo(ResourceState newState) = 0;

        /// @brief Returns the current resource state.
        [[nodiscard]] virtual ResourceState GetCurrentState() const noexcept = 0;
    };

    // ========================================================================
    // ITexture2D
    // ========================================================================

    /// @brief A 2D texture — the most common texture type.
    class ITexture2D : public virtual ITexture
    {
    public:
        /// @brief Convenience: upload a single 2D image to mip 0, layer 0.
        virtual void Upload(const void* pixels, usize size,
                            u32 width, u32 height) = 0;
    };

    // ========================================================================
    // ITextureCube
    // ========================================================================

    /// @brief A cubemap texture — six 2D faces arranged as array layers.
    class ITextureCube : public virtual ITexture
    {
    public:
        /// @brief Uploads pixel data for a specific cubemap face.
        /// @param face  Face index 0..5 (+X, -X, +Y, -Y, +Z, -Z).
        virtual void UploadFace(u32 face, u32 mipLevel, const TextureData& data) = 0;
    };

    // ========================================================================
    // ITextureArray
    // ========================================================================

    /// @brief A 2D texture array — multiple 2D textures sharing the same
    ///        format and dimensions, addressable by a layer index in the
    ///        shader.
    class ITextureArray : public virtual ITexture
    {
    public:
        /// @brief Uploads pixel data for a specific layer.
        virtual void UploadLayer(u32 arrayLayer, u32 mipLevel, const TextureData& data) = 0;
    };

    // ========================================================================
    // ISampler
    // ========================================================================

    /// @brief Sampler — controls how a texture is filtered when sampled
    ///        from a shader.  Immutable after creation.
    class ISampler : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual const SamplerDescription& GetDescription() const noexcept = 0;

        [[nodiscard]] virtual FilterMode  GetMinFilter() const noexcept = 0;
        [[nodiscard]] virtual FilterMode  GetMagFilter() const noexcept = 0;
        [[nodiscard]] virtual FilterMode  GetMipFilter() const noexcept = 0;
        [[nodiscard]] virtual AddressMode GetAddressU()  const noexcept = 0;
        [[nodiscard]] virtual AddressMode GetAddressV()  const noexcept = 0;
        [[nodiscard]] virtual AddressMode GetAddressW()  const noexcept = 0;
        [[nodiscard]] virtual bool        IsAnisotropyEnabled() const noexcept = 0;
        [[nodiscard]] virtual f32         GetMaxAnisotropy() const noexcept = 0;
    };

} // namespace engine::rhi
