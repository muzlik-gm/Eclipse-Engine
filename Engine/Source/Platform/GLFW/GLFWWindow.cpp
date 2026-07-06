// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWWindow.cpp
// GLFW-backed implementation of the IWindow interface.
// All GLFW #includes are isolated to this translation unit.
// ============================================================================

#include "Engine/Platform/Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/Log.h"

#define GLFW_INCLUDE_NONE
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

    using namespace engine::events;

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
                ENGINE_LOG_WARN("GLFWWindow — window already created");
                return;
            }

            m_Properties = props;

            // Request an OpenGL context (not GLFW_NO_API which is for Vulkan).
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_SAMPLES, 0);
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
                ENGINE_LOG_ERROR("GLFWWindow — failed to create GLFW window");
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

            ENGINE_LOG_INFO("GLFWWindow — created '{}' ({}x{})",
                            m_Properties.Title,
                            m_Properties.Width,
                            m_Properties.Height);

            WindowCreatedEvent createdEvent(m_Properties.Width, m_Properties.Height);
            if (m_EventCallback) m_EventCallback(createdEvent);
        }

        void Destroy() override
        {
            if (!m_Window) return;

            ENGINE_LOG_INFO("GLFWWindow — destroying '{}'", m_Properties.Title);
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

    std::unique_ptr<IWindow> IWindow::CreateWindow(const WindowProperties& props)
    {
        auto window = std::make_unique<GLFWWindow>();
        window->Create(props);
        return window;
    }

} // namespace engine::platform
