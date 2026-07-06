// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLSync.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/ISync.h"

#include <glad/glad.h>
#include <memory>
#include <string>
#include <vector>

namespace engine::opengl {

    using engine::core::u32;
    using engine::core::u64;
    using engine::rhi::ISampler;
    using engine::rhi::ITexture;
    using engine::rhi::IUniformBuffer;
    using engine::rhi::IStorageBuffer;
    using engine::rhi::IDescriptorLayout;
    using engine::rhi::IDescriptorSet;
    using engine::rhi::IDescriptorPool;
    using engine::rhi::DescriptorType;
    using engine::rhi::DescriptorLayoutDescription;

    class OpenGLDebugLayer;

    // ========================================================================
    // OpenGLFence
    // ========================================================================

    class OpenGLFence final : public engine::rhi::IFence
    {
    public:
        OpenGLFence(bool signaled, OpenGLDebugLayer* debugLayer);
        ~OpenGLFence() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override { m_Name = std::string(name); }
        engine::core::u64 GetNativeHandle() const noexcept override { return reinterpret_cast<engine::core::u64>(m_Sync); }
        bool IsValid() const noexcept override { return m_Sync != nullptr; }

        void Reset() override;
        bool IsSignaled() const override;
        void Wait() override;
        engine::core::u64 GetValue() const noexcept override { return m_Signaled ? 1 : 0; }

    protected:
        bool WaitForNanoseconds(engine::core::u64 timeoutNs) override;

    private:
        GLsync             m_Sync{nullptr};
        bool               m_Signaled{false};
        std::string        m_Name;
        OpenGLDebugLayer*  m_DebugLayer{nullptr};
    };

    // ========================================================================
    // OpenGLSemaphore
    // ========================================================================

    class OpenGLSemaphore final : public engine::rhi::ISemaphore
    {
    public:
        OpenGLSemaphore(engine::core::u64 initialValue, OpenGLDebugLayer* debugLayer);
        ~OpenGLSemaphore() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override { m_Name = std::string(name); }
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        engine::core::u64 GetValue() const noexcept override { return m_Value; }
        void Reset(engine::core::u64 value = 0) override { m_Value = value; }

    private:
        engine::core::u64  m_Value{0};
        std::string        m_Name;
        OpenGLDebugLayer*  m_DebugLayer{nullptr};
    };

    // ========================================================================
    // OpenGLDescriptorLayout / Set / Pool
    // ========================================================================

    class OpenGLDescriptorLayout final : public engine::rhi::IDescriptorLayout
    {
    public:
        OpenGLDescriptorLayout(const engine::rhi::DescriptorLayoutDescription& desc)
            : m_Description(desc) {}

        std::string_view GetDebugName() const noexcept override { return "DescLayout"; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        const engine::rhi::DescriptorLayoutDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::core::u32 GetBindingCount() const noexcept override
        { return static_cast<engine::core::u32>(m_Description.Bindings.size()); }

        engine::rhi::DescriptorType GetBindingType(engine::core::u32 binding) const override
        {
            for (const auto& b : m_Description.Bindings)
                if (b.Binding == binding) return b.Type;
            return engine::rhi::DescriptorType::None;
        }

        engine::core::u32 GetBindingCount(engine::core::u32 binding) const override
        {
            for (const auto& b : m_Description.Bindings)
                if (b.Binding == binding) return b.Count;
            return 0;
        }

    private:
        engine::rhi::DescriptorLayoutDescription m_Description;
    };

    class OpenGLDescriptorSet final : public engine::rhi::IDescriptorSet
    {
    public:
        explicit OpenGLDescriptorSet(OpenGLDescriptorLayout* layout) : m_Layout(layout) {}

        std::string_view GetDebugName() const noexcept override { return "DescSet"; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        engine::rhi::IDescriptorLayout* GetLayout() const noexcept override { return m_Layout; }

        void BindSampler(engine::core::u32 binding, engine::rhi::ISampler* sampler) override;
        void BindTexture(engine::core::u32 binding, engine::rhi::ITexture* texture) override;
        void BindUniformBuffer(engine::core::u32 binding, engine::rhi::IUniformBuffer* buffer) override;
        void BindStorageBuffer(engine::core::u32 binding, engine::rhi::IStorageBuffer* buffer) override;
        void Update() override {}

    private:
        OpenGLDescriptorLayout* m_Layout;
    };

    class OpenGLDescriptorPool final : public engine::rhi::IDescriptorPool
    {
    public:
        OpenGLDescriptorPool(engine::core::u32 maxSets,
                             const std::vector<engine::rhi::DescriptorType>& /*allowedTypes*/)
            : m_MaxSets(maxSets) {}

        std::string_view GetDebugName() const noexcept override { return "DescPool"; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        engine::rhi::IDescriptorSet* Allocate(engine::rhi::IDescriptorLayout* layout) override;
        void Reset() override;
        engine::core::u32 GetAllocatedCount() const noexcept override { return m_Allocated; }
        engine::core::u32 GetMaxSets() const noexcept override { return m_MaxSets; }

    private:
        engine::core::u32 m_MaxSets{0};
        engine::core::u32 m_Allocated{0};
        std::vector<std::unique_ptr<OpenGLDescriptorSet>> m_Sets;
    };

} // namespace engine::opengl
