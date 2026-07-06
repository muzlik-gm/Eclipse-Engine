// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLAdapter.h
// ============================================================================
#pragma once

#include "Engine/Renderer/RHI/Interfaces/IGraphicsDevice.h"
#include "OpenGLCapabilities.h"
#include <string>

namespace engine::opengl {

    class OpenGLAdapter final : public engine::rhi::IGraphicsAdapter
    {
    public:
        OpenGLAdapter() = default;
        ~OpenGLAdapter() override = default;

        // -- IGraphicsObject
        std::string_view GetDebugName() const noexcept override { return m_DeviceName; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        // -- ICapabilities
        engine::rhi::GraphicsBackend GetBackend() const noexcept override
        { return m_Caps.GetBackend(); }
        engine::rhi::GraphicsVendor GetVendor() const noexcept override
        { return m_Caps.GetVendor(); }
        const std::string& GetDeviceName() const noexcept override
        { return m_Caps.GetDeviceName(); }
        const std::string& GetDriverVersion() const noexcept override
        { return m_Caps.GetDriverVersion(); }
        const std::string& GetAPIVersion() const noexcept override
        { return m_Caps.GetAPIVersion(); }
        const engine::rhi::AdapterLimits& GetLimits() const noexcept override
        { return m_Caps.GetLimits(); }
        const engine::rhi::AdapterFeatures& GetFeatures() const noexcept override
        { return m_Caps.GetFeatures(); }
        engine::rhi::FormatProperties GetFormatProperties(engine::rhi::GraphicsFormat f) const override
        { return m_Caps.GetFormatProperties(f); }
        bool IsFormatSupported(engine::rhi::GraphicsFormat f, engine::rhi::TextureUsage u) const override
        { return m_Caps.IsFormatSupported(f, u); }
        engine::core::u32 GetMaxSampleCount() const noexcept override
        { return m_Caps.GetMaxSampleCount(); }
        engine::core::u32 GetMaxAnisotropy() const noexcept override
        { return m_Caps.GetMaxAnisotropy(); }

        // -- IGraphicsAdapter
        const std::string& GetAdapterUUID() const noexcept override { return m_UUID; }
        engine::core::u32 GetDeviceID() const noexcept override { return 0; }
        engine::core::u64 GetDedicatedVideoMemory() const noexcept override { return 0; }
        engine::core::u64 GetSharedSystemMemory() const noexcept override { return 0; }

    private:
        OpenGLCapabilities m_Caps;
        std::string        m_DeviceName;
        std::string        m_UUID;
    };

} // namespace engine::opengl
