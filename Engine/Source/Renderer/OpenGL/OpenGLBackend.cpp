// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLBackend.cpp
// ============================================================================
#include "OpenGLBackend.h"
#include "OpenGLDevice.h"
#include "Engine/Core/Log.h"

namespace engine::opengl {

    using namespace engine::rhi;

    // ========================================================================
    // OpenGLGraphicsInstance — wraps the OpenGL device.
    // ========================================================================

    class OpenGLGraphicsInstance final : public IGraphicsInstance
    {
    public:
        explicit OpenGLGraphicsInstance(std::unique_ptr<OpenGLDevice> device)
            : m_Device(std::move(device)) {}

        ~OpenGLGraphicsInstance() override = default;

        std::string_view GetDebugName() const noexcept override { return "OpenGLInstance"; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return m_Device && m_Device->IsValid(); }

        GraphicsBackend GetBackend() const noexcept override
        { return GraphicsBackend::OpenGL; }

        std::vector<IGraphicsAdapter*> EnumerateAdapters() override
        {
            return {m_Device ? m_Device->GetAdapter() : nullptr};
        }

        IGraphicsAdapter* GetAdapter() const noexcept override
        { return m_Device ? m_Device->GetAdapter() : nullptr; }

        IGraphicsDevice* GetDevice() const noexcept override
        { return m_Device.get(); }

    private:
        std::unique_ptr<OpenGLDevice> m_Device;
    };

    // ========================================================================
    // OpenGLBackend
    // ========================================================================

    OpenGLBackend::OpenGLBackend() = default;

    bool OpenGLBackend::IsAvailable() const
    {
        // OpenGL is available wherever a context can be created.
        return true;
    }

    std::unique_ptr<IGraphicsInstance>
        OpenGLBackend::CreateInstance(const BackendCreateInfo& info)
    {
        ENGINE_LOG_INFO("OpenGLBackend — creating instance");

        auto device = std::make_unique<OpenGLDevice>();
        if (!device->Initialize(static_cast<GLFWwindow*>(info.WindowHandle),
                                 info.EnableDebugOutput))
        {
            ENGINE_LOG_ERROR("OpenGLBackend — device initialization failed");
            return nullptr;
        }

        return std::make_unique<OpenGLGraphicsInstance>(std::move(device));
    }

    // ========================================================================
    // Self-registration
    // ========================================================================

    static OpenGLBackend       g_OpenGLBackend;
    static engine::rhi::BackendRegistrar g_OpenGLBackendRegistrar(&g_OpenGLBackend);

} // namespace engine::opengl
