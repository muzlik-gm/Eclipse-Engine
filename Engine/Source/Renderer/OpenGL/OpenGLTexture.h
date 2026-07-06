// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLTexture.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/ITexture.h"
#include <glad/glad.h>
#include <string>

namespace engine::opengl {

    class OpenGLDebugLayer;

    // ========================================================================
    // OpenGLTexture — base texture implementing ITexture.
    // ========================================================================

    class OpenGLTexture : public virtual engine::rhi::ITexture
    {
    public:
        OpenGLTexture(const engine::rhi::TextureDescription& desc,
                      OpenGLDebugLayer* debugLayer);
        ~OpenGLTexture() override;

        OpenGLTexture(const OpenGLTexture&)            = delete;
        OpenGLTexture& operator=(const OpenGLTexture&) = delete;
        OpenGLTexture(OpenGLTexture&&)                 = delete;
        OpenGLTexture& operator=(OpenGLTexture&&)      = delete;

        // -- IGraphicsObject
        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override;
        engine::core::u64 GetNativeHandle() const noexcept override { return m_Handle; }
        bool IsValid() const noexcept override { return m_Handle != 0; }

        // -- ITexture
        const engine::rhi::TextureDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::rhi::TextureType   GetType() const noexcept override { return m_Description.Type; }
        engine::rhi::TextureFormat GetFormat() const noexcept override { return m_Description.Format; }
        engine::rhi::TextureUsage  GetUsage() const noexcept override { return m_Description.Usage; }

        engine::core::u32 GetWidth()       const noexcept override { return m_Description.Width; }
        engine::core::u32 GetHeight()      const noexcept override { return m_Description.Height; }
        engine::core::u32 GetDepth()       const noexcept override { return m_Description.Depth; }
        engine::core::u32 GetMipLevels()   const noexcept override { return m_Description.MipLevels; }
        engine::core::u32 GetArrayLayers() const noexcept override { return m_Description.ArrayLayers; }
        engine::core::u32 GetSamples()     const noexcept override { return m_Description.Samples; }

        void UploadData(engine::core::u32 mipLevel, engine::core::u32 arrayLayer,
                        const engine::rhi::TextureData& data,
                        engine::core::u32 xOffset = 0, engine::core::u32 yOffset = 0,
                        engine::core::u32 width = 0, engine::core::u32 height = 0) override;

        void GenerateMipmaps() override;
        void TransitionTo(engine::rhi::ResourceState newState) override;
        engine::rhi::ResourceState GetCurrentState() const noexcept override;

        // -- OpenGL-specific
        [[nodiscard]] GLuint GetHandle() const noexcept { return m_Handle; }
        [[nodiscard]] GLuint GetTarget() const noexcept { return m_Target; }
        void Bind(engine::core::u32 slot);

    protected:
        engine::rhi::TextureDescription m_Description;
        GLuint                           m_Handle{0};
        GLuint                           m_Target{GL_TEXTURE_2D};
        std::string                      m_Name;
        OpenGLDebugLayer*                m_DebugLayer{nullptr};
        engine::rhi::ResourceState       m_CurrentState{engine::rhi::ResourceState::Sampled};
    };

    // ========================================================================
    // OpenGLTexture2D — implements ITexture2D via inheritance from OpenGLTexture.
    // ========================================================================

    class OpenGLTexture2D final : public OpenGLTexture, public engine::rhi::ITexture2D
    {
    public:
        OpenGLTexture2D(const engine::rhi::TextureDescription& desc,
                        OpenGLDebugLayer* debugLayer);
        void Upload(const void* pixels, engine::core::usize size,
                    engine::core::u32 width, engine::core::u32 height) override;
    };

    // ========================================================================
    // OpenGLTextureCube
    // ========================================================================

    class OpenGLTextureCube final : public OpenGLTexture, public engine::rhi::ITextureCube
    {
    public:
        OpenGLTextureCube(const engine::rhi::TextureDescription& desc,
                          OpenGLDebugLayer* debugLayer);
        void UploadFace(engine::core::u32 face, engine::core::u32 mipLevel,
                        const engine::rhi::TextureData& data) override;
    };

    // ========================================================================
    // OpenGLTextureArray
    // ========================================================================

    class OpenGLTextureArray final : public OpenGLTexture, public engine::rhi::ITextureArray
    {
    public:
        OpenGLTextureArray(const engine::rhi::TextureDescription& desc,
                           OpenGLDebugLayer* debugLayer);
        void UploadLayer(engine::core::u32 arrayLayer, engine::core::u32 mipLevel,
                         const engine::rhi::TextureData& data) override;
    };

    // ========================================================================
    // OpenGLSampler
    // ========================================================================

    class OpenGLSampler final : public engine::rhi::ISampler
    {
    public:
        OpenGLSampler(const engine::rhi::SamplerDescription& desc,
                      OpenGLDebugLayer* debugLayer);
        ~OpenGLSampler() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override;
        engine::core::u64 GetNativeHandle() const noexcept override { return m_Handle; }
        bool IsValid() const noexcept override { return m_Handle != 0; }

        const engine::rhi::SamplerDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::rhi::FilterMode  GetMinFilter() const noexcept override { return m_Description.MinFilter; }
        engine::rhi::FilterMode  GetMagFilter() const noexcept override { return m_Description.MagFilter; }
        engine::rhi::FilterMode  GetMipFilter() const noexcept override { return m_Description.MipFilter; }
        engine::rhi::AddressMode GetAddressU()  const noexcept override { return m_Description.AddressU; }
        engine::rhi::AddressMode GetAddressV()  const noexcept override { return m_Description.AddressV; }
        engine::rhi::AddressMode GetAddressW()  const noexcept override { return m_Description.AddressW; }
        bool IsAnisotropyEnabled() const noexcept override { return m_Description.AnisotropyEnable; }
        engine::core::f32 GetMaxAnisotropy() const noexcept override { return m_Description.MaxAnisotropy; }

        [[nodiscard]] GLuint GetHandle() const noexcept { return m_Handle; }
        void Bind(engine::core::u32 slot);

    private:
        engine::rhi::SamplerDescription m_Description;
        GLuint                           m_Handle{0};
        std::string                      m_Name;
        OpenGLDebugLayer*                m_DebugLayer{nullptr};
    };

} // namespace engine::opengl
