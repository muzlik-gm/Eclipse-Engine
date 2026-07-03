// ============================================================================
// File: Engine/Include/Engine/Platform/PlatformManager.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Platform/Clipboard.h"
#include "Engine/Platform/Monitor.h"
#include "Engine/Platform/PlatformInfo.h"
#include "Engine/Platform/Window.h"

#include <memory>
#include <vector>

namespace engine::platform {

    // ========================================================================
    // PlatformManager – central manager for the platform subsystem
    // ========================================================================
    ///
    /// Owns the lifetime of the underlying platform library (e.g. GLFW),
    /// and serves as the factory for windows, clipboard, and monitor queries.
    /// Managed by the Application class; users typically interact with it
    /// through Application::GetPlatformManager().
    ///
    class PlatformManager : private engine::core::NonCopyable
    {
    public:
        PlatformManager();
        ~PlatformManager();

        PlatformManager(PlatformManager&&) noexcept;
        PlatformManager& operator=(PlatformManager&&) noexcept;

        // --------------------------------------------------------------------
        // Lifecycle
        // --------------------------------------------------------------------

        /// Initializes the platform subsystem (e.g. calls glfwInit).
        /// Safe to call multiple times; subsequent calls are no-ops.
        void Initialize();

        /// Shuts down the platform subsystem and releases all resources.
        void Shutdown();

        // --------------------------------------------------------------------
        // Factory methods
        // --------------------------------------------------------------------

        /// Creates a new platform window with the given properties.
        [[nodiscard]] std::unique_ptr<IWindow> CreateWindow(const WindowProperties& props);

        // --------------------------------------------------------------------
        // Monitor queries
        // --------------------------------------------------------------------

        /// Returns information about all connected monitors.
        [[nodiscard]] std::vector<MonitorInfo> GetMonitors() const;

        /// Returns information about the primary monitor.
        [[nodiscard]] MonitorInfo GetPrimaryMonitor() const;

        // --------------------------------------------------------------------
        // Clipboard
        // --------------------------------------------------------------------

        /// Returns a reference to the clipboard interface.
        [[nodiscard]] IClipboard& GetClipboard();

        // --------------------------------------------------------------------
        // Platform information
        // --------------------------------------------------------------------

        /// Returns a snapshot of the current platform information.
        [[nodiscard]] PlatformInfo GetPlatformInfo() const;

        // --------------------------------------------------------------------
        // State queries
        // --------------------------------------------------------------------

        /// Returns true if the platform subsystem has been initialized.
        [[nodiscard]] bool IsInitialized() const;

        // --------------------------------------------------------------------
        // Singleton accessor
        // --------------------------------------------------------------------

        /// Returns the single PlatformManager instance (Meyers singleton).
        [[nodiscard]] static PlatformManager& Instance();

    private:
        bool m_Initialized = false;
        std::unique_ptr<IClipboard> m_Clipboard;
    };

} // namespace engine::platform