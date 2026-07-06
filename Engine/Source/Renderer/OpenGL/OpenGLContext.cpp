// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLContext.cpp
// ============================================================================
#include "OpenGLContext.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace engine::opengl {

    using engine::core::u64;

    OpenGLContext::OpenGLContext() = default;

    OpenGLContext::~OpenGLContext()
    {
        Destroy();
    }

    bool OpenGLContext::Create(GLFWwindow* window, bool enableDebug)
    {
        if (!window)
        {
            ENGINE_LOG_ERROR("OpenGLContext — null GLFW window");
            return false;
        }

        m_Window = window;

        glfwMakeContextCurrent(window);

        // Initialize GLAD once.
        static bool gladLoaded = false;
        if (!gladLoaded)
        {
            const int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
            if (version == 0)
            {
                ENGINE_LOG_ERROR("OpenGLContext — gladLoadGLLoader failed");
                return false;
            }
            gladLoaded = true;
            ENGINE_LOG_INFO("OpenGLContext — GLAD loaded");
        }

        // Query OpenGL version from the driver.
        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        ENGINE_LOG_INFO("OpenGLContext — driver reports OpenGL {}.{}", major, minor);

        if (major < 4 || (major == 4 && minor < 6))
        {
            ENGINE_LOG_WARN("OpenGLContext — OpenGL {}.{} detected, 4.6 recommended",
                            major, minor);
        }

        // Enable vsync by default.
        glfwSwapInterval(1);
        m_VSync = true;

        // Print some driver info.
        const GLubyte* vendor   = glGetString(GL_VENDOR);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version  = glGetString(GL_VERSION);
        const GLubyte* glsl     = glGetString(GL_SHADING_LANGUAGE_VERSION);

        ENGINE_LOG_INFO("OpenGLContext — Vendor:   {}", reinterpret_cast<const char*>(vendor));
        ENGINE_LOG_INFO("OpenGLContext — Renderer: {}", reinterpret_cast<const char*>(renderer));
        ENGINE_LOG_INFO("OpenGLContext — Version:  {}", reinterpret_cast<const char*>(version));
        ENGINE_LOG_INFO("OpenGLContext — GLSL:     {}", reinterpret_cast<const char*>(glsl));

        return true;
    }

    void OpenGLContext::Destroy()
    {
        // Context is destroyed when the GLFW window is destroyed.
        m_Window = nullptr;
    }

    u64 OpenGLContext::GetNativeHandle() const noexcept
    {
        return reinterpret_cast<u64>(m_Window);
    }

    void OpenGLContext::MakeCurrent()
    {
        if (m_Window)
            glfwMakeContextCurrent(m_Window);
    }

    void OpenGLContext::ReleaseCurrent()
    {
        glfwMakeContextCurrent(nullptr);
    }

    bool OpenGLContext::IsCurrent() const noexcept
    {
        return m_Window != nullptr && glfwGetCurrentContext() == m_Window;
    }

    void OpenGLContext::SwapBuffers()
    {
        if (m_Window)
            glfwSwapBuffers(m_Window);
    }

    void OpenGLContext::SetVSync(bool enabled)
    {
        m_VSync = enabled;
        if (m_Window)
            glfwSwapInterval(enabled ? 1 : 0);
    }

} // namespace engine::opengl
