// ============================================================================
// File: Engine/Include/Engine/Platform/Window.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Events/Event.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace engine::platform {

    using engine::core::u8;
    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;
    using engine::core::f64;

    // ========================================================================
    // WindowMode
    // ========================================================================
    enum class WindowMode
    {
        Windowed           = 0,
        Fullscreen         = 1,
        BorderlessFullscreen = 2
    };

    // ========================================================================
    // CursorMode
    // ========================================================================
    enum class CursorMode
    {
        Normal  = 0,
        Hidden  = 1,
        Locked  = 2
    };

    // ========================================================================
    // WindowProperties
    // ========================================================================
    struct WindowProperties
    {
        std::string Title         = "Engine Window";
        u32         Width         = 1280;
        u32         Height        = 720;
        WindowMode  Mode          = WindowMode::Windowed;
        bool        Resizable     = true;
        bool        VSync         = false;
        u32         MinWidth      = 0;
        u32         MinHeight     = 0;
        u32         MaxWidth      = 0;
        u32         MaxHeight     = 0;
        f32         Opacity       = 1.0f;
        bool        VisibleOnCreate = true;
    };

    // ========================================================================
    // IWindow – pure-virtual platform window abstraction
    // ========================================================================
    class IWindow : private engine::core::NonCopyable
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
        virtual void Minimize() = 0;
        virtual void Maximize() = 0;
        virtual void Restore() = 0;

        // --------------------------------------------------------------------
        // Properties
        // --------------------------------------------------------------------
        virtual void SetTitle(const std::string& title) = 0;
        virtual void SetSize(u32 width, u32 height) = 0;
        virtual void SetPosition(i32 x, i32 y) = 0;
        virtual void SetOpacity(f32 opacity) = 0;

        [[nodiscard]] virtual u32              GetWidth() const = 0;
        [[nodiscard]] virtual u32              GetHeight() const = 0;
        [[nodiscard]] virtual void             GetPosition(i32& outX, i32& outY) const = 0;
        [[nodiscard]] virtual const std::string& GetTitle() const = 0;

        // --------------------------------------------------------------------
        // Main loop
        // --------------------------------------------------------------------
        [[nodiscard]] virtual bool ShouldClose() const = 0;
        virtual void PollEvents() = 0;
        virtual void SwapBuffers() = 0;

        // --------------------------------------------------------------------
        // VSync
        // --------------------------------------------------------------------
        virtual void SetVSync(bool enabled) = 0;
        [[nodiscard]] virtual bool IsVSync() const = 0;

        // --------------------------------------------------------------------
        // Fullscreen
        // --------------------------------------------------------------------
        virtual void SetFullscreenMode(WindowMode mode) = 0;
        [[nodiscard]] virtual WindowMode GetFullscreenMode() const = 0;

        // --------------------------------------------------------------------
        // Focus & State queries
        // --------------------------------------------------------------------
        virtual void Focus() = 0;
        [[nodiscard]] virtual bool IsFocused() const = 0;
        [[nodiscard]] virtual bool IsMinimized() const = 0;
        [[nodiscard]] virtual bool IsMaximized() const = 0;
        [[nodiscard]] virtual bool IsVisible() const = 0;

        // --------------------------------------------------------------------
        // Cursor
        // --------------------------------------------------------------------
        virtual void SetCursorPosition(f64 x, f64 y) = 0;
        virtual void GetCursorPosition(f64& outX, f64& outY) const = 0;
        virtual void SetCursorMode(CursorMode mode) = 0;
        [[nodiscard]] virtual CursorMode GetCursorMode() const = 0;

        // --------------------------------------------------------------------
        // Clipboard
        // --------------------------------------------------------------------
        virtual void        SetClipboardText(const std::string& text) = 0;
        virtual std::string GetClipboardText() const = 0;

        // --------------------------------------------------------------------
        // Icon – iconData is raw RGBA pixels
        // --------------------------------------------------------------------
        virtual void SetIcon(const std::vector<u8>& iconData, u32 width, u32 height) = 0;

        // --------------------------------------------------------------------
        // Native handle
        // --------------------------------------------------------------------
        [[nodiscard]] virtual void* GetNativeHandle() const = 0;

        // --------------------------------------------------------------------
        // Graphics context
        // --------------------------------------------------------------------
        virtual void MakeContextCurrent() = 0;

        // --------------------------------------------------------------------
        // Event callback
        // --------------------------------------------------------------------
        using EventCallbackFn = std::function<void(events::Event&)>;
        virtual void SetEventCallback(EventCallbackFn callback) = 0;

        // --------------------------------------------------------------------
        // Factory – creates the GLFW-backed implementation.
        // --------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<IWindow> CreateWindow(const WindowProperties& props);
    };

} // namespace engine::platform