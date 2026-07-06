// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/ICapabilities.h
// Abstract interface for querying graphics adapter capabilities, format
// support, and feature/limit information.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <array>
#include <string>
#include <vector>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::f32;

    // ========================================================================
    // FormatProperties — per-format support information.
    // ========================================================================

    /// @brief Describes how a format can be used on a given adapter.
    struct FormatProperties
    {
        GraphicsFormat Format{GraphicsFormat::Undefined};

        bool SupportsSampled{false};
        bool SupportsStorage{false};
        bool SupportsRenderTarget{false};
        bool SupportsDepthStencil{false};
        bool SupportsBlend{false};
        bool SupportsTransfer{false};

        /// True if the format requires a sampler to access.
        bool RequiresSampler{true};

        /// Optimal tiling mode hint (backend-specific; opaque to the RHI).
        u32 OptimalTiling{0};
    };

    // ========================================================================
    // AdapterLimits — hard limits of the graphics adapter.
    // ========================================================================

    /// @brief Reports the hard limits of a graphics adapter.
    struct AdapterLimits
    {
        u32 MaxTexture1DSize{0};
        u32 MaxTexture2DSize{0};
        u32 MaxTexture3DSize{0};
        u32 MaxTextureCubeSize{0};
        u32 MaxTextureArrayLayers{0};
        u32 MaxTextureMipLevels{0};

        u32 MaxUniformBufferSize{0};
        u32 MaxStorageBufferSize{0};
        u32 MaxUniformBufferRange{0};
        u32 MaxStorageBufferRange{0};

        u32 MaxVertexInputAttributes{0};
        u32 MaxVertexInputBindings{0};
        u32 MaxVertexOutputComponents{0};

        u32 MaxColorAttachments{0};
        u32 MaxFramebufferWidth{0};
        u32 MaxFramebufferHeight{0};
        u32 MaxFramebufferLayers{0};

        u32 MaxComputeSharedMemorySize{0};
        u32 MaxComputeWorkGroupCountX{0};
        u32 MaxComputeWorkGroupCountY{0};
        u32 MaxComputeWorkGroupCountZ{0};
        u32 MaxComputeWorkGroupSizeX{0};
        u32 MaxComputeWorkGroupSizeY{0};
        u32 MaxComputeWorkGroupSizeZ{0};

        u32 MaxSamplerAnisotropy{0};
        u32 MaxSamplerLodBias{0};

        u32 MaxViewportWidth{0};
        u32 MaxViewportHeight{0};
        u32 MaxViewports{0};

        u32 MaxPushConstantsSize{0};

        u32 MaxDescriptorSetSamplers{0};
        u32 MaxDescriptorSetSampledImages{0};
        u32 MaxDescriptorSetStorageImages{0};
        u32 MaxDescriptorSetUniformBuffers{0};
        u32 MaxDescriptorSetStorageBuffers{0};
    };

    // ========================================================================
    // AdapterFeatures — boolean feature flags.
    // ========================================================================

    /// @brief Reports the optional features supported by a graphics adapter.
    struct AdapterFeatures
    {
        bool SupportsGeometryShader{false};
        bool SupportsTessellationShader{false};
        bool SupportsComputeShader{false};
        bool SupportsMeshShader{false};
        bool SupportsRayTracing{false};

        bool SupportsAnisotropicFiltering{false};
        bool SupportsTextureCompressionBC{false};
        bool SupportsTextureCompressionETC2{false};
        bool SupportsTextureCompressionASTC{false};

        bool SupportsMultiDrawIndirect{false};
        bool SupportsDrawIndirectFirstInstance{false};

        bool SupportsPipelineStatisticsQuery{false};
        bool SupportsOcclusionQuery{false};
        bool SupportsTimestampQuery{false};

        bool SupportsWireframeFill{false};
        bool SupportsDepthClamp{false};
        bool SupportsDepthBias{false};

        bool SupportsIndependentBlend{false};
        bool SupportsDualSourceBlend{false};
        bool SupportsLogicOp{false};

        bool SupportsPersistentMapping{false};
        bool SupportsMultiSampledFramebuffers{false};

        bool SupportsDebugMarkers{false};
        bool SupportsDebugOutput{false};
        bool SupportsPushConstants{false};
        bool SupportsDescriptorIndexing{false};
    };

    // ========================================================================
    // ICapabilities — interface for querying adapter capabilities.
    // ========================================================================

    /// @brief Interface for querying adapter capabilities, format support,
    ///        and feature/limit information.  Implemented by the backend
    ///        after adapter creation.
    class ICapabilities
    {
    public:
        virtual ~ICapabilities() = default;

        /// @brief Returns the active graphics backend.
        [[nodiscard]] virtual GraphicsBackend GetBackend() const noexcept = 0;

        /// @brief Returns the GPU vendor.
        [[nodiscard]] virtual GraphicsVendor GetVendor() const noexcept = 0;

        /// @brief Returns the GPU device name (e.g. "NVIDIA GeForce RTX 4090").
        [[nodiscard]] virtual const std::string& GetDeviceName() const noexcept = 0;

        /// @brief Returns the driver version string.
        [[nodiscard]] virtual const std::string& GetDriverVersion() const noexcept = 0;

        /// @brief Returns the API version string (e.g. "4.6.0").
        [[nodiscard]] virtual const std::string& GetAPIVersion() const noexcept = 0;

        /// @brief Returns the adapter's hard limits.
        [[nodiscard]] virtual const AdapterLimits& GetLimits() const noexcept = 0;

        /// @brief Returns the adapter's optional features.
        [[nodiscard]] virtual const AdapterFeatures& GetFeatures() const noexcept = 0;

        /// @brief Returns the format properties for a given format.
        [[nodiscard]] virtual FormatProperties GetFormatProperties(GraphicsFormat format) const = 0;

        /// @brief Returns true if a format is supported for the given usage.
        [[nodiscard]] virtual bool IsFormatSupported(GraphicsFormat format, TextureUsage usage) const = 0;

        /// @brief Returns the maximum supported MSAA sample count.
        [[nodiscard]] virtual u32 GetMaxSampleCount() const noexcept = 0;

        /// @brief Returns the maximum anisotropy value supported.
        [[nodiscard]] virtual u32 GetMaxAnisotropy() const noexcept = 0;
    };

} // namespace engine::rhi
