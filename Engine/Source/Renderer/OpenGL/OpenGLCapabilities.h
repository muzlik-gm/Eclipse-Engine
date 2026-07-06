// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLCapabilities.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/ICapabilities.h"
#include <string>

namespace engine::opengl {

    class OpenGLCapabilities final : public engine::rhi::ICapabilities
    {
    public:
        OpenGLCapabilities();
        ~OpenGLCapabilities() override = default;

        void Initialize();

        // -- ICapabilities
        engine::rhi::GraphicsBackend GetBackend() const noexcept override;
        engine::rhi::GraphicsVendor GetVendor() const noexcept override;
        const std::string& GetDeviceName() const noexcept override;
        const std::string& GetDriverVersion() const noexcept override;
        const std::string& GetAPIVersion() const noexcept override;
        const engine::rhi::AdapterLimits& GetLimits() const noexcept override;
        const engine::rhi::AdapterFeatures& GetFeatures() const noexcept override;
        engine::rhi::FormatProperties GetFormatProperties(engine::rhi::GraphicsFormat format) const override;
        bool IsFormatSupported(engine::rhi::GraphicsFormat format,
                               engine::rhi::TextureUsage usage) const override;
        engine::core::u32 GetMaxSampleCount() const noexcept override;
        engine::core::u32 GetMaxAnisotropy() const noexcept override;

    private:
        void QueryVendor();
        void QueryLimits();
        void QueryFeatures();

        engine::rhi::GraphicsVendor  m_Vendor{engine::rhi::GraphicsVendor::Unknown};
        std::string                  m_DeviceName;
        std::string                  m_DriverVersion;
        std::string                  m_APIVersion;
        engine::rhi::AdapterLimits   m_Limits{};
        engine::rhi::AdapterFeatures m_Features{};
        engine::core::u32            m_MaxSamples{1};
        engine::core::u32            m_MaxAnisotropy{1};
    };

} // namespace engine::opengl
