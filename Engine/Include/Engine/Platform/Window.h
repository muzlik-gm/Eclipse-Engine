// ============================================================================
// File: Engine/Include/Engine/Platform/Window.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <functional>
#include <memory>
#include <string>

namespace engine::platform {

    using engine::core::u32;

    /// Properties used to configure a window at creation time.
    struct WindowProperties
    {
        std::string Title  = "Engine Window";
        u32         Width  = 1280;
        u32         Height = 720;
        bool        Fullscreen = false;
        bool        VSync      = false;
        bool        Resizable  = true;
    };

    /// Callback signature for platform-level event dispatching.
    /// Receives an Event reference (forward-declared here; full definition in Event.h).
    /// Callers typically wrap this with a typed event dispatcher.
    using EventCallback = std::function<void(void*)>;

    /// Pure-virtual platform window abstraction.
    /// Concrete implementations are provided per platform (Win32, X11, Wayland, …).
    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        // --------------------------------------------------------------------
        // Lifecycle
        // --------------------------------------------------------------------
        virtual void Create(const WindowProperties& props) = 0;
        virtual void Destroy() = 0;
        virtual void Show() = 0;
        virtual void Hide() = 0;

        // --------------------------------------------------------------------
        // State
        // --------------------------------------------------------------------
        virtual void Resize(u32 width, u32 height) = 0;
        virtual void SetTitle(const std::string& title) = 0;
        virtual bool ShouldClose() const = 0;
        virtual void PollEvents() = 0;
        virtual void SwapBuffers() = 0;

        // --------------------------------------------------------------------
        // Factory – creates the platform-specific implementation.
        // --------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<IWindow> Create(const WindowProperties& props);
    };

} // namespace engine::platform