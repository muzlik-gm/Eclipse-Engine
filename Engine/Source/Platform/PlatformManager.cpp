// ============================================================================
// File: Engine/Source/Platform/PlatformManager.cpp
// Implementation of the PlatformManager — GLFW lifecycle, window factory,
// monitor enumeration, and platform information.
// ============================================================================

#include "Engine/Platform/PlatformManager.h"
#include "Engine/Core/Log.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <utility>

namespace engine::platform
{

    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    PlatformManager::PlatformManager()  = default;

    PlatformManager::~PlatformManager()
    {
        Shutdown();
    }

    PlatformManager::PlatformManager(PlatformManager&& other) noexcept
        : m_Initialized(std::exchange(other.m_Initialized, false))
        , m_Clipboard(std::move(other.m_Clipboard))
    {}

    PlatformManager& PlatformManager::operator=(PlatformManager&& other) noexcept
    {
        if (this != &other)
        {
            Shutdown();
            m_Initialized = std::exchange(other.m_Initialized, false);
            m_Clipboard   = std::move(other.m_Clipboard);
        }
        return *this;
    }

    // ========================================================================
    // Lifecycle
    // ========================================================================

    void PlatformManager::Initialize()
    {
        if (m_Initialized)
        {
            return;
        }

        ENGINE_LOG_INFO("PlatformManager — initializing platform subsystem (GLFW)");

        if (!glfwInit())
        {
            ENGINE_LOG_CRITICAL("PlatformManager — glfwInit() failed");
            return;
        }

        glfwSetErrorCallback([](int error, const char* description)
        {
            ENGINE_LOG_ERROR("GLFW error {} ({})", error, description);
        });

        m_Clipboard = IClipboard::Create();
        m_Initialized = true;

        ENGINE_LOG_INFO("PlatformManager — initialized successfully (GLFW {})",
                        glfwGetVersionString());
    }

    void PlatformManager::Shutdown()
    {
        if (!m_Initialized)
        {
            return;
        }

        ENGINE_LOG_INFO("PlatformManager — shutting down platform subsystem");

        m_Clipboard.reset();
        glfwTerminate();

        m_Initialized = false;
        ENGINE_LOG_INFO("PlatformManager — shutdown complete");
    }

    // ========================================================================
    // Factory
    // ========================================================================

    std::unique_ptr<IWindow> PlatformManager::CreateWindow(const WindowProperties& props)
    {
        if (!m_Initialized)
        {
            ENGINE_LOG_ERROR("PlatformManager — cannot create window: not initialized");
            return nullptr;
        }

        return IWindow::CreateWindow(props);
    }

    // ========================================================================
    // Monitor queries
    // ========================================================================

    std::vector<MonitorInfo> PlatformManager::GetMonitors() const
    {
        if (!m_Initialized)
        {
            ENGINE_LOG_WARN("PlatformManager — cannot enumerate monitors: not initialized");
            return {};
        }
        return IMonitor::EnumerateMonitors();
    }

    MonitorInfo PlatformManager::GetPrimaryMonitor() const
    {
        if (!m_Initialized)
        {
            ENGINE_LOG_WARN("PlatformManager — cannot get primary monitor: not initialized");
            return MonitorInfo{};
        }
        return IMonitor::GetPrimaryMonitor();
    }

    // ========================================================================
    // Clipboard
    // ========================================================================

    IClipboard& PlatformManager::GetClipboard()
    {
        return *m_Clipboard;
    }

    // ========================================================================
    // Platform information
    // ========================================================================

    PlatformInfo PlatformManager::GetPlatformInfo() const
    {
        return PlatformInfo::Gather();
    }

    // ========================================================================
    // State
    // ========================================================================

    bool PlatformManager::IsInitialized() const
    {
        return m_Initialized;
    }

    // ========================================================================
    // Singleton
    // ========================================================================

    PlatformManager& PlatformManager::Instance()
    {
        static PlatformManager instance;
        return instance;
    }

} // namespace engine::platform
