#!/usr/bin/env python3
"""Generate all Phase 2 GLFW implementation source files."""

import os

BASE = "/home/z/my-project/Engine/Source/Platform"

files = {}

# ============================================================================
# GLFWWindow.cpp
# ============================================================================
files[os.path.join(BASE, "GLFW", "GLFWWindow.cpp")] = '''// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWWindow.cpp
// GLFW-backed implementation of the IWindow interface.
// All GLFW #includes are isolated to this translation unit.
// ============================================================================

#include "Engine/Platform/Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/Log.h"

#include <GLFW/glfw3.h>

#include <string>
#include <utility>

namespace engine::platform
{

    using engine::core::f64;
    using engine::core::f32;
    using engine::core::u8;
    using engine::core::u32;
    using engine::core::i32;

    // ========================================================================
    // GLFWWindow
    // ========================================================================

    class GLFWWindow final : public IWindow
    {
    public:
        GLFWWindow()  = default;
        ~GLFWWindow() override { Destroy(); }

        GLFWWindow(const GLFWWindow&)            = delete;
        GLFWWindow& operator=(const GLFWWindow&) = delete;
        GLFWWindow(GLFWWindow&&)                 = delete;
        GLFWWindow& operator=(GLFWWindow&&)      = delete;

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------
        void Create(const WindowProperties& props) override
        {
            if (m_Window)
            {
                ENGINE_LOG_WARN("GLFWWindow \u2014 window already created");
                return;
            }

            m_Properties = props;

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, m_Properties.Resizable ? GLFW_TRUE : GLFW_FALSE);
            glfwWindowHint(GLFW_VISIBLE, m_Properties.VisibleOnCreate ? GLFW_TRUE : GLFW_FALSE);
            glfwWindowHint(GLFW_DECORATED,
                            m_Properties.Mode == WindowMode::BorderlessFullscreen
                                ? GLFW_FALSE : GLFW_TRUE);

            if (m_Properties.Mode == WindowMode::Fullscreen)
            {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                m_Window = glfwCreateWindow(
                    static_cast<int>(mode->width),
                    static_cast<int>(mode->height),
                    m_Properties.Title.c_str(),
                    monitor,
                    nullptr);
            }
            else if (m_Properties.Mode == WindowMode::BorderlessFullscreen)
            {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
                glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
                m_Window = glfwCreateWindow(
                    static_cast<int>(mode->width),
                    static_cast<int>(mode->height),
                    m_Properties.Title.c_str(),
                    monitor,
                    nullptr);
            }
            else
            {
                m_Window = glfwCreateWindow(
                    static_cast<int>(m_Properties.Width),
                    static_cast<int>(m_Properties.Height),
                    m_Properties.Title.c_str(),
                    nullptr,
                    nullptr);
            }

            if (!m_Window)
            {
                ENGINE_LOG_ERROR("GLFWWindow \u2014 failed to create GLFW window");
                return;
            }

            if (m_Properties.MinWidth > 0 && m_Properties.MinHeight > 0)
            {
                glfwSetWindowSizeLimits(m_Window,
                    static_cast<int>(m_Properties.MinWidth),
                    static_cast<int>(m_Properties.MinHeight),
                    static_cast<int>(m_Properties.MaxWidth > 0 ? m_Properties.MaxWidth : GLFW_DONT_CARE),
                    static_cast<int>(m_Properties.MaxHeight > 0 ? m_Properties.MaxHeight : GLFW_DONT_CARE));
            }

            glfwSetWindowUserPointer(m_Window, this);

            glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* w)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                WindowCloseEvent event;
                if (self->m_EventCallback) self->m_EventCallback(event);
            });

            glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* w, int width, int height)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                self->m_Properties.Width  = static_cast<u32>(width);
                self->m_Properties.Height = static_cast<u32>(height);
                WindowResizeEvent event(static_cast<u32>(width), static_cast<u32>(height));
                if (self->m_EventCallback) self->m_EventCallback(event);
            });

            glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* w, int width, int height)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                FramebufferResizeEvent event(static_cast<u32>(width), static_cast<u32>(height));
                if (self->m_EventCallback) self->m_EventCallback(event);
            });

            glfwSetWindowPosCallback(m_Window, [](GLFWwindow* w, int x, int y)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                WindowMovedEvent event(x, y);
                if (self->m_EventCallback) self->m_EventCallback(event);
            });

            glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* w, int focused)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                if (focused)
                {
                    WindowFocusEvent event;
                    if (self->m_EventCallback) self->m_EventCallback(event);
                }
                else
                {
                    WindowLostFocusEvent event;
                    if (self->m_EventCallback) self->m_EventCallback(event);
                }
            });

            glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* w, int iconified)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                if (iconified)
                {
                    WindowMinimizedEvent event;
                    if (self->m_EventCallback) self->m_EventCallback(event);
                }
                else
                {
                    WindowRestoredEvent event;
                    if (self->m_EventCallback) self->m_EventCallback(event);
                }
            });

            glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow* w, int maximized)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                if (maximized)
                {
                    WindowMaximizedEvent event;
                    if (self->m_EventCallback) self->m_EventCallback(event);
                }
                else
                {
                    WindowRestoredEvent event;
                    if (self->m_EventCallback) self->m_EventCallback(event);
                }
            });

            glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow* w)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                WindowRefreshEvent event;
                if (self->m_EventCallback) self->m_EventCallback(event);
            });

            glfwSetWindowContentScaleCallback(m_Window, [](GLFWwindow* w, float sx, float sy)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(w));
                DPIChangedEvent event(static_cast<f32>(sx), static_cast<f32>(sy));
                if (self->m_EventCallback) self->m_EventCallback(event);
            });

            if (m_Properties.Opacity < 1.0f)
            {
                SetOpacity(m_Properties.Opacity);
            }

            ENGINE_LOG_INFO("GLFWWindow \u2014 created '{}' ({}x{})",
                            m_Properties.Title,
                            m_Properties.Width,
                            m_Properties.Height);

            WindowCreatedEvent createdEvent(m_Properties.Width, m_Properties.Height);
            if (m_EventCallback) m_EventCallback(createdEvent);
        }

        void Destroy() override
        {
            if (!m_Window) return;

            ENGINE_LOG_INFO("GLFWWindow \u2014 destroying '{}'", m_Properties.Title);
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }

        void Show() override
        {
            if (m_Window) glfwShowWindow(m_Window);
        }

        void Hide() override
        {
            if (m_Window) glfwHideWindow(m_Window);
        }

        void Minimize() override
        {
            if (m_Window) glfwIconifyWindow(m_Window);
        }

        void Maximize() override
        {
            if (m_Window) glfwMaximizeWindow(m_Window);
        }

        void Restore() override
        {
            if (m_Window) glfwRestoreWindow(m_Window);
        }

        // ----------------------------------------------------------------
        // Properties
        // ----------------------------------------------------------------
        void SetTitle(const std::string& title) override
        {
            m_Properties.Title = title;
            if (m_Window) glfwSetWindowTitle(m_Window, title.c_str());
        }

        void SetSize(u32 width, u32 height) override
        {
            m_Properties.Width  = width;
            m_Properties.Height = height;
            if (m_Window) glfwSetWindowSize(m_Window, static_cast<int>(width), static_cast<int>(height));
        }

        void SetPosition(i32 x, i32 y) override
        {
            if (m_Window) glfwSetWindowPos(m_Window, x, y);
        }

        void SetOpacity(f32 opacity) override
        {
            m_Properties.Opacity = opacity;
            if (m_Window) glfwSetWindowOpacity(m_Window, static_cast<double>(opacity));
        }

        [[nodiscard]] u32 GetWidth() const override
        {
            if (m_Window)
            {
                int w = 0;
                glfwGetWindowSize(m_Window, &w, nullptr);
                return static_cast<u32>(w);
            }
            return m_Properties.Width;
        }

        [[nodiscard]] u32 GetHeight() const override
        {
            if (m_Window)
            {
                int h = 0;
                glfwGetWindowSize(m_Window, nullptr, &h);
                return static_cast<u32>(h);
            }
            return m_Properties.Height;
        }

        void GetPosition(i32& outX, i32& outY) const override
        {
            if (m_Window)
            {
                int x = 0, y = 0;
                glfwGetWindowPos(m_Window, &x, &y);
                outX = x;
                outY = y;
                return;
            }
            outX = 0;
            outY = 0;
        }

        [[nodiscard]] const std::string& GetTitle() const override
        {
            return m_Properties.Title;
        }

        // ----------------------------------------------------------------
        // Main loop
        // ----------------------------------------------------------------
        [[nodiscard]] bool ShouldClose() const override
        {
            return m_Window ? (glfwWindowShouldClose(m_Window) != 0) : true;
        }

        void PollEvents() override
        {
            glfwPollEvents();
        }

        void SwapBuffers() override
        {
            // No-op without a graphics context.  Rendering backends will
            // provide their own swap mechanism in future phases.
        }

        // ----------------------------------------------------------------
        // VSync
        // ----------------------------------------------------------------
        void SetVSync(bool enabled) override
        {
            m_Properties.VSync = enabled;
        }

        [[nodiscard]] bool IsVSync() const override
        {
            return m_Properties.VSync;
        }

        // ----------------------------------------------------------------
        // Fullscreen
        // ----------------------------------------------------------------
        void SetFullscreenMode(WindowMode mode) override
        {
            if (!m_Window) return;

            if (mode == WindowMode::Windowed)
            {
                glfwSetWindowMonitor(m_Window, nullptr,
                    m_SavedWindowedPos[0], m_SavedWindowedPos[1],
                    m_SavedWindowedSize[0], m_SavedWindowedSize[1], GLFW_DONT_CARE);
                m_Properties.Mode = WindowMode::Windowed;
            }
            else if (mode == WindowMode::Fullscreen)
            {
                glfwGetWindowPos(m_Window, &m_SavedWindowedPos[0], &m_SavedWindowedPos[1]);
                glfwGetWindowSize(m_Window, &m_SavedWindowedSize[0], &m_SavedWindowedSize[1]);

                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(m_Window, monitor, 0, 0,
                    vidmode->width, vidmode->height, vidmode->refreshRate);
                m_Properties.Mode = WindowMode::Fullscreen;
            }
            else if (mode == WindowMode::BorderlessFullscreen)
            {
                glfwGetWindowPos(m_Window, &m_SavedWindowedPos[0], &m_SavedWindowedPos[1]);
                glfwGetWindowSize(m_Window, &m_SavedWindowedSize[0], &m_SavedWindowedSize[1]);

                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(m_Window, monitor, 0, 0,
                    vidmode->width, vidmode->height, vidmode->refreshRate);
                m_Properties.Mode = WindowMode::BorderlessFullscreen;
            }
        }

        [[nodiscard]] WindowMode GetFullscreenMode() const override
        {
            return m_Properties.Mode;
        }

        // ----------------------------------------------------------------
        // Focus & State
        // ----------------------------------------------------------------
        void Focus() override
        {
            if (m_Window) glfwFocusWindow(m_Window);
        }

        [[nodiscard]] bool IsFocused() const override
        {
            return m_Window ? (glfwGetWindowAttrib(m_Window, GLFW_FOCUSED) != 0) : false;
        }

        [[nodiscard]] bool IsMinimized() const override
        {
            return m_Window ? (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) != 0) : false;
        }

        [[nodiscard]] bool IsMaximized() const override
        {
            return m_Window ? (glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED) != 0) : false;
        }

        [[nodiscard]] bool IsVisible() const override
        {
            return m_Window ? (glfwGetWindowAttrib(m_Window, GLFW_VISIBLE) != 0) : false;
        }

        // ----------------------------------------------------------------
        // Cursor
        // ----------------------------------------------------------------
        void SetCursorPosition(f64 x, f64 y) override
        {
            if (m_Window) glfwSetCursorPos(m_Window, x, y);
        }

        void GetCursorPosition(f64& outX, f64& outY) const override
        {
            if (m_Window)
            {
                glfwGetCursorPos(m_Window, &outX, &outY);
                return;
            }
            outX = 0.0;
            outY = 0.0;
        }

        void SetCursorMode(CursorMode mode) override
        {
            m_CursorMode = mode;
            if (!m_Window) return;

            int glfwMode = GLFW_CURSOR_NORMAL;
            switch (mode)
            {
                case CursorMode::Normal:  glfwMode = GLFW_CURSOR_NORMAL;  break;
                case CursorMode::Hidden:  glfwMode = GLFW_CURSOR_HIDDEN;  break;
                case CursorMode::Locked:  glfwMode = GLFW_CURSOR_DISABLED; break;
            }
            glfwSetInputMode(m_Window, GLFW_CURSOR, glfwMode);
        }

        [[nodiscard]] CursorMode GetCursorMode() const override
        {
            return m_CursorMode;
        }

        // ----------------------------------------------------------------
        // Clipboard
        // ----------------------------------------------------------------
        void SetClipboardText(const std::string& text) override
        {
            if (m_Window) glfwSetClipboardString(m_Window, text.c_str());
        }

        std::string GetClipboardText() const override
        {
            if (!m_Window) return "";
            const char* text = glfwGetClipboardString(m_Window);
            return text ? std::string(text) : "";
        }

        // ----------------------------------------------------------------
        // Icon
        // ----------------------------------------------------------------
        void SetIcon(const std::vector<u8>& iconData, u32 width, u32 height) override
        {
            if (!m_Window || iconData.empty()) return;

            GLFWimage image;
            image.width  = static_cast<int>(width);
            image.height = static_cast<int>(height);
            image.pixels = const_cast<unsigned char*>(iconData.data());
            glfwSetWindowIcon(m_Window, 1, &image);
        }

        // ----------------------------------------------------------------
        // Native handle
        // ----------------------------------------------------------------
        [[nodiscard]] void* GetNativeHandle() const override
        {
            return static_cast<void*>(m_Window);
        }

        // ----------------------------------------------------------------
        // Graphics context
        // ----------------------------------------------------------------
        void MakeContextCurrent() override
        {
            if (m_Window) glfwMakeContextCurrent(m_Window);
        }

        // ----------------------------------------------------------------
        // Event callback
        // ----------------------------------------------------------------
        void SetEventCallback(EventCallbackFn callback) override
        {
            m_EventCallback = std::move(callback);
        }

    private:
        GLFWwindow*        m_Window  = nullptr;
        WindowProperties   m_Properties;
        CursorMode         m_CursorMode = CursorMode::Normal;
        EventCallbackFn    m_EventCallback;
        int                m_SavedWindowedPos[2]  = {0, 0};
        int                m_SavedWindowedSize[2] = {0, 0};
    };

    // ========================================================================
    // IWindow factory
    // ========================================================================

    std::unique_ptr<IWindow> IWindow::Create(const WindowProperties& props)
    {
        auto window = std::make_unique<GLFWWindow>();
        window->Create(props);
        return window;
    }

} // namespace engine::platform
'''

files[os.path.join(BASE, "GLFW", "GLFWMonitor.cpp")] = '''// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWMonitor.cpp
// GLFW-backed implementation of monitor enumeration and queries.
// ============================================================================

#include "Engine/Platform/Monitor.h"
#include "Engine/Core/Log.h"

#include <GLFW/glfw3.h>

namespace engine::platform
{

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;
    using engine::core::u8;
    using engine::core::usize;

    // ========================================================================
    // Helper \u2014 convert GLFWvidmode to VideoMode
    // ========================================================================

    static VideoMode GLFWVideoModeToEngine(const GLFWvidmode& mode)
    {
        VideoMode vm;
        vm.Width       = static_cast<u32>(mode.width);
        vm.Height      = static_cast<u32>(mode.height);
        vm.RefreshRate = mode.refreshRate;
        vm.RedBits     = static_cast<u8>(mode.redBits);
        vm.GreenBits   = static_cast<u8>(mode.greenBits);
        vm.BlueBits    = static_cast<u8>(mode.blueBits);
        return vm;
    }

    // ========================================================================
    // Helper \u2014 gather info for one monitor
    // ========================================================================

    static MonitorInfo GatherMonitorInfo(GLFWmonitor* monitor, bool isPrimary)
    {
        MonitorInfo info;
        info.IsPrimary = isPrimary;

        const char* name = glfwGetMonitorName(monitor);
        info.Name = name ? std::string(name) : "Unknown Monitor";

        int xpos = 0, ypos = 0;
        glfwGetMonitorPos(monitor, &xpos, &ypos);
        info.PositionX = xpos;
        info.PositionY = ypos;

        int widthMM = 0, heightMM = 0;
        glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
        info.PhysicalWidthMM  = static_cast<u32>(widthMM);
        info.PhysicalHeightMM = static_cast<u32>(heightMM);

        float sx = 1.0f, sy = 1.0f;
        glfwGetMonitorContentScale(monitor, &sx, &sy);
        info.ContentScaleX = static_cast<f32>(sx);
        info.ContentScaleY = static_cast<f32>(sy);

        int wx = 0, wy = 0, ww = 0, wh = 0;
        glfwGetMonitorWorkarea(monitor, &wx, &wy, &ww, &wh);
        info.WorkAreaX      = wx;
        info.WorkAreaY      = wy;
        info.WorkAreaWidth  = static_cast<u32>(ww);
        info.WorkAreaHeight = static_cast<u32>(wh);

        const GLFWvidmode* currentMode = glfwGetVideoMode(monitor);
        if (currentMode)
        {
            info.CurrentMode = GLFWVideoModeToEngine(*currentMode);
        }

        int modeCount = 0;
        const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
        info.VideoModes.reserve(static_cast<usize>(modeCount));
        for (int i = 0; i < modeCount; ++i)
        {
            info.VideoModes.push_back(GLFWVideoModeToEngine(modes[i]));
        }

        return info;
    }

    // ========================================================================
    // Static queries
    // ========================================================================

    std::vector<MonitorInfo> IMonitor::EnumerateMonitors()
    {
        std::vector<MonitorInfo> result;

        int count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        if (!monitors || count <= 0)
        {
            ENGINE_LOG_WARN("GLFWMonitor \u2014 no monitors found");
            return result;
        }

        GLFWmonitor* primary = glfwGetPrimaryMonitor();

        result.reserve(static_cast<usize>(count));
        for (int i = 0; i < count; ++i)
        {
            bool isPrimary = (monitors[i] == primary);
            result.push_back(GatherMonitorInfo(monitors[i], isPrimary));
        }

        return result;
    }

    MonitorInfo IMonitor::GetPrimaryMonitor()
    {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (!primary)
        {
            ENGINE_LOG_WARN("GLFWMonitor \u2014 no primary monitor found");
            return MonitorInfo{};
        }

        return GatherMonitorInfo(primary, true);
    }

} // namespace engine::platform
'''

files[os.path.join(BASE, "GLFW", "GLFWClipboard.cpp")] = '''// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWClipboard.cpp
// GLFW-backed implementation of the IClipboard interface.
// ============================================================================

#include "Engine/Platform/Clipboard.h"
#include "Engine/Core/Log.h"

#include <GLFW/glfw3.h>

namespace engine::platform
{

    // ========================================================================
    // GLFWClipboard
    // ========================================================================

    class GLFWClipboard final : public IClipboard
    {
    public:
        explicit GLFWClipboard(GLFWwindow* window)
            : m_Window(window)
        {}

        void SetText(const std::string& text) override
        {
            if (m_Window)
            {
                glfwSetClipboardString(m_Window, text.c_str());
            }
        }

        [[nodiscard]] std::string GetText() const override
        {
            if (!m_Window) return "";
            const char* text = glfwGetClipboardString(m_Window);
            return text ? std::string(text) : "";
        }

        void Clear() override
        {
            if (m_Window)
            {
                glfwSetClipboardString(m_Window, "");
            }
        }

        [[nodiscard]] bool HasText() const override
        {
            if (!m_Window) return false;
            const char* text = glfwGetClipboardString(m_Window);
            return text != nullptr && text[0] != \'\\0\';
        }

    private:
        GLFWwindow* m_Window = nullptr;
    };

    // ========================================================================
    // IClipboard factory
    // ========================================================================

    std::unique_ptr<IClipboard> IClipboard::Create()
    {
        return std::make_unique<GLFWClipboard>(nullptr);
    }

} // namespace engine::platform
'''

files[os.path.join(BASE, "GLFW", "GLFWCursor.cpp")] = '''// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWCursor.cpp
// GLFW-backed implementation of the ICursor interface.
// ============================================================================

#include "Engine/Platform/Cursor.h"
#include "Engine/Core/Log.h"

#include <GLFW/glfw3.h>

namespace engine::platform
{

    using engine::core::f64;

    // ========================================================================
    // Helper \u2014 map CursorShape to GLFW standard cursor
    // ========================================================================

    static GLFWcursor* CreateStandardCursor(CursorShape shape)
    {
        int glfwShape = GLFW_ARROW_CURSOR;
        switch (shape)
        {
            case CursorShape::Arrow:     glfwShape = GLFW_ARROW_CURSOR;     break;
            case CursorShape::IBeam:     glfwShape = GLFW_IBEAM_CURSOR;     break;
            case CursorShape::Crosshair: glfwShape = GLFW_CROSSHAIR_CURSOR; break;
            case CursorShape::Hand:      glfwShape = GLFW_HAND_CURSOR;      break;
            case CursorShape::HResize:   glfwShape = GLFW_HRESIZE_CURSOR;  break;
            case CursorShape::VResize:   glfwShape = GLFW_VRESIZE_CURSOR;  break;
            case CursorShape::ResizeAll: glfwShape = GLFW_RESIZE_ALL_CURSOR; break;
            case CursorShape::NoCursor:  return nullptr;
        }
        return glfwCreateStandardCursor(glfwShape);
    }

    // ========================================================================
    // GLFWCursor
    // ========================================================================

    class GLFWCursor final : public ICursor
    {
    public:
        explicit GLFWCursor(GLFWwindow* window)
            : m_Window(window)
        {}

        ~GLFWCursor() override
        {
            if (m_CurrentCursor)
            {
                glfwDestroyCursor(m_CurrentCursor);
            }
        }

        void SetShape(CursorShape shape) override
        {
            m_Shape = shape;

            if (m_CurrentCursor)
            {
                glfwDestroyCursor(m_CurrentCursor);
                m_CurrentCursor = nullptr;
            }

            if (shape == CursorShape::NoCursor)
            {
                if (m_Window) glfwSetCursor(m_Window, nullptr);
                return;
            }

            m_CurrentCursor = CreateStandardCursor(shape);
            if (m_CurrentCursor && m_Window)
            {
                glfwSetCursor(m_Window, m_CurrentCursor);
            }
        }

        [[nodiscard]] CursorShape GetShape() const override
        {
            return m_Shape;
        }

        [[nodiscard]] bool IsVisible() const override
        {
            return m_Visible && m_Shape != CursorShape::NoCursor;
        }

        void SetVisible(bool visible) override
        {
            m_Visible = visible;
            if (!m_Window) return;

            if (visible)
            {
                SetShape(m_Shape);
            }
            else
            {
                glfwSetCursor(m_Window, nullptr);
            }
        }

        void SetPosition(f64 x, f64 y) override
        {
            if (m_Window) glfwSetCursorPos(m_Window, x, y);
        }

        void GetPosition(f64& outX, f64& outY) const override
        {
            if (m_Window)
            {
                glfwGetCursorPos(m_Window, &outX, &outY);
                return;
            }
            outX = 0.0;
            outY = 0.0;
        }

        void EnableLockMode() override
        {
            m_Locked = true;
            if (m_Window) glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        void DisableLockMode() override
        {
            m_Locked = false;
            if (m_Window) glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        [[nodiscard]] bool IsLocked() const override
        {
            return m_Locked;
        }

    private:
        GLFWwindow* m_Window         = nullptr;
        GLFWcursor* m_CurrentCursor   = nullptr;
        CursorShape m_Shape           = CursorShape::Arrow;
        bool        m_Visible         = true;
        bool        m_Locked          = false;
    };

    // ========================================================================
    // ICursor factory
    // ========================================================================

    std::unique_ptr<ICursor> ICursor::Create(void* windowNativeHandle)
    {
        auto* glfwWindow = static_cast<GLFWwindow*>(windowNativeHandle);
        return std::make_unique<GLFWCursor>(glfwWindow);
    }

} // namespace engine::platform
'''

files[os.path.join(BASE, "GLFW", "GLFWDynamicLibrary.cpp")] = '''// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWDynamicLibrary.cpp
// Platform-specific implementation of IDynamicLibrary using dlopen/dlsym.
// ============================================================================

#include "Engine/Platform/DynamicLibrary.h"
#include "Engine/Core/Log.h"

#ifdef ENGINE_PLATFORM_LINUX
    #include <dlfcn.h>
#elif defined(ENGINE_PLATFORM_MACOS)
    #include <dlfcn.h>
#elif defined(ENGINE_PLATFORM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

namespace engine::platform
{

    // ========================================================================
    // PlatformDynamicLibrary
    // ========================================================================

    class PlatformDynamicLibrary final : public IDynamicLibrary
    {
    public:
        PlatformDynamicLibrary()  = default;
        ~PlatformDynamicLibrary() override { Unload(); }

        PlatformDynamicLibrary(const PlatformDynamicLibrary&)            = delete;
        PlatformDynamicLibrary& operator=(const PlatformDynamicLibrary&) = delete;
        PlatformDynamicLibrary(PlatformDynamicLibrary&&)                 = delete;
        PlatformDynamicLibrary& operator=(PlatformDynamicLibrary&&)      = delete;

        bool Load(const std::string& filePath) override
        {
            if (m_Handle)
            {
                ENGINE_LOG_WARN("DynamicLibrary \u2014 already loaded \'{}\', unloading first", m_FilePath);
                Unload();
            }

#ifdef ENGINE_PLATFORM_LINUX
            m_Handle = dlopen(filePath.c_str(), RTLD_NOW);
#elif defined(ENGINE_PLATFORM_MACOS)
            m_Handle = dlopen(filePath.c_str(), RTLD_NOW);
#elif defined(ENGINE_PLATFORM_WINDOWS)
            m_Handle = static_cast<void*>(LoadLibraryA(filePath.c_str()));
#endif

            if (!m_Handle)
            {
#ifdef ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
                ENGINE_LOG_ERROR("DynamicLibrary \u2014 failed to load \'{}\': {}", filePath, dlerror());
#elif defined(ENGINE_PLATFORM_WINDOWS)
                ENGINE_LOG_ERROR("DynamicLibrary \u2014 failed to load \'{}\'", filePath);
#endif
                return false;
            }

            m_FilePath = filePath;
            ENGINE_LOG_DEBUG("DynamicLibrary \u2014 loaded \'{}\'", filePath);
            return true;
        }

        void Unload() override
        {
            if (!m_Handle) return;

#ifdef ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
            dlclose(m_Handle);
#elif defined(ENGINE_PLATFORM_WINDOWS)
            FreeLibrary(static_cast<HMODULE>(m_Handle));
#endif

            ENGINE_LOG_DEBUG("DynamicLibrary \u2014 unloaded \'{}\'", m_FilePath);
            m_Handle   = nullptr;
            m_FilePath.clear();
        }

        [[nodiscard]] void* GetSymbol(const std::string& name) const override
        {
            if (!m_Handle) return nullptr;

#ifdef ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
            return dlsym(m_Handle, name.c_str());
#elif defined(ENGINE_PLATFORM_WINDOWS)
            return static_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_Handle), name.c_str()));
#else
            return nullptr;
#endif
        }

        [[nodiscard]] bool IsLoaded() const override
        {
            return m_Handle != nullptr;
        }

        [[nodiscard]] const std::string& GetFilePath() const override
        {
            return m_FilePath;
        }

    private:
        void*       m_Handle   = nullptr;
        std::string m_FilePath;
    };

    // ========================================================================
    // IDynamicLibrary factory
    // ========================================================================

    std::unique_ptr<IDynamicLibrary> IDynamicLibrary::Create()
    {
        return std::make_unique<PlatformDynamicLibrary>();
    }

} // namespace engine::platform
'''

files[os.path.join(BASE, "PlatformManager.cpp")] = '''// ============================================================================
// File: Engine/Source/Platform/PlatformManager.cpp
// Implementation of the PlatformManager \u2014 GLFW lifecycle, window factory,
// monitor enumeration, and platform information.
// ============================================================================

#include "Engine/Platform/PlatformManager.h"
#include "Engine/Core/Log.h"

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

        ENGINE_LOG_INFO("PlatformManager \u2014 initializing platform subsystem (GLFW)");

        if (!glfwInit())
        {
            ENGINE_LOG_CRITICAL("PlatformManager \u2014 glfwInit() failed");
            return;
        }

        glfwSetErrorCallback([](int error, const char* description)
        {
            ENGINE_LOG_ERROR("GLFW error {} ({})", error, description);
        });

        m_Clipboard = IClipboard::Create();
        m_Initialized = true;

        ENGINE_LOG_INFO("PlatformManager \u2014 initialized successfully (GLFW {})",
                        glfwGetVersionString());
    }

    void PlatformManager::Shutdown()
    {
        if (!m_Initialized)
        {
            return;
        }

        ENGINE_LOG_INFO("PlatformManager \u2014 shutting down platform subsystem");

        m_Clipboard.reset();
        glfwTerminate();

        m_Initialized = false;
        ENGINE_LOG_INFO("PlatformManager \u2014 shutdown complete");
    }

    // ========================================================================
    // Factory
    // ========================================================================

    std::unique_ptr<IWindow> PlatformManager::CreateWindow(const WindowProperties& props)
    {
        if (!m_Initialized)
        {
            ENGINE_LOG_ERROR("PlatformManager \u2014 cannot create window: not initialized");
            return nullptr;
        }

        return IWindow::Create(props);
    }

    // ========================================================================
    // Monitor queries
    // ========================================================================

    std::vector<MonitorInfo> PlatformManager::GetMonitors() const
    {
        if (!m_Initialized)
        {
            ENGINE_LOG_WARN("PlatformManager \u2014 cannot enumerate monitors: not initialized");
            return {};
        }
        return IMonitor::EnumerateMonitors();
    }

    MonitorInfo PlatformManager::GetPrimaryMonitor() const
    {
        if (!m_Initialized)
        {
            ENGINE_LOG_WARN("PlatformManager \u2014 cannot get primary monitor: not initialized");
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
'''

files[os.path.join(BASE, "PlatformInfo.cpp")] = '''// ============================================================================
// File: Engine/Source/Platform/PlatformInfo.cpp
// Implementation of PlatformInfo static queries.
// ============================================================================

#include "Engine/Platform/PlatformInfo.h"
#include "Engine/Diagnostics/SystemInfo.h"

#include <cstdio>
#include <string>

namespace engine::platform
{

    std::string PlatformInfo::GetOSName()
    {
        return diagnostics::SystemInfo::OSName();
    }

    std::string PlatformInfo::GetOSVersion()
    {
#if defined(ENGINE_PLATFORM_LINUX)
        FILE* f = std::fopen("/proc/version", "r");
        if (f)
        {
            char buf[256] = {};
            if (std::fgets(buf, sizeof(buf), f))
            {
                std::fclose(f);
                std::string str(buf);
                auto pos = str.find("Linux version ");
                if (pos != std::string::npos)
                {
                    auto start = pos + 15;
                    auto end = str.find(' ', start);
                    if (end != std::string::npos)
                    {
                        return str.substr(start, end - start);
                    }
                }
                return str;
            }
            std::fclose(f);
        }
        return "Unknown";
#elif defined(ENGINE_PLATFORM_MACOS)
        return diagnostics::SystemInfo::OSName();
#elif defined(ENGINE_PLATFORM_WINDOWS)
        return diagnostics::SystemInfo::OSName();
#else
        return "Unknown";
#endif
    }

    std::string PlatformInfo::GetArchitecture()
    {
        return diagnostics::SystemInfo::Architecture();
    }

    std::string PlatformInfo::GetPlatformName()
    {
#if defined(ENGINE_PLATFORM_LINUX)
        return "Linux";
#elif defined(ENGINE_PLATFORM_MACOS)
        return "macOS";
#elif defined(ENGINE_PLATFORM_WINDOWS)
        return "Windows";
#else
        return "Unknown";
#endif
    }

    std::string PlatformInfo::GetEnginePlatformString()
    {
        return GetPlatformName() + " " + GetArchitecture() + " | " + GetOSName();
    }

    bool PlatformInfo::Is64Bit()
    {
        return sizeof(void*) == 8;
    }

    PlatformInfo PlatformInfo::Gather()
    {
        PlatformInfo info;
        info.OSName         = GetOSName();
        info.OSVersion      = GetOSVersion();
        info.Architecture    = GetArchitecture();
        info.PlatformName    = GetPlatformName();
        info.Is64BitPlatform = Is64Bit();
        return info;
    }

} // namespace engine::platform
'''

# Write all files
for path, content in files.items():
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write(content)
    print(f"Written: {path}")

print(f"\nTotal files written: {len(files)}")