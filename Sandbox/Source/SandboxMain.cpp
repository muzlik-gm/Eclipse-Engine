// ============================================================================
// File: Sandbox/Source/SandboxMain.cpp
// Sandbox application that renders a colored triangle using the engine's
// platform layer + OpenGL backend.  Runs until the window is closed.
// ============================================================================

#include "Engine/Core/Types.h"
#include "Engine/Core/Log.h"
#include "Engine/Application/Application.h"
#include "Engine/Runtime/Engine.h"
#include "Engine/Runtime/ISubsystem.h"
#include "Engine/Platform/Window.h"
#include "Engine/Platform/PlatformInfo.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <functional>
#include <string>

using namespace engine;
using engine::core::f64;
using engine::core::u64;

// ========================================================================
// RenderingSubsystem — renders a colored triangle every frame.
// ========================================================================

class RenderingSubsystem final : public runtime::ISubsystem
{
public:
    explicit RenderingSubsystem(application::Application* app)
        : m_app(app)
    {
    }

    [[nodiscard]] std::string_view GetName() const noexcept override
    { return "Rendering"; }

    bool Initialize() override
    {
        ENGINE_LOG_INFO("[Rendering] Initialize()");

        // Get the GLFW window and make the GL context current.
        auto* window = m_app->GetWindow();
        if (!window)
        {
            ENGINE_LOG_ERROR("[Rendering] No window available");
            return false;
        }

        m_glfwWindow = static_cast<GLFWwindow*>(window->GetNativeHandle());
        glfwMakeContextCurrent(m_glfwWindow);

        // Initialize GLAD.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            ENGINE_LOG_ERROR("[Rendering] Failed to initialize GLAD");
            return false;
        }

        ENGINE_LOG_INFO("[Rendering] OpenGL {} — {}",
                        reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                        reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

        // Enable vsync.
        glfwSwapInterval(1);

        // Compile shaders.
        if (!CompileShaders())
        {
            ENGINE_LOG_ERROR("[Rendering] Shader compilation failed");
            return false;
        }

        // Create vertex buffer + vertex array for the triangle.
        CreateTriangle();

        ENGINE_LOG_INFO("[Rendering] Initialized — rendering a colored triangle");
        ENGINE_LOG_INFO("[Rendering] Close the window or press ESC to exit.");

        return true;
    }

    void Shutdown() override
    {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_Program) glDeleteProgram(m_Program);
        ENGINE_LOG_INFO("[Rendering] Shutdown()");
    }

    void Update(f64 deltaTime) override
    {
        ++m_frameCount;
        m_elapsed += deltaTime;

        // Check for window close or ESC.
        if (glfwWindowShouldClose(m_glfwWindow))
        {
            m_app->GetEngine().RequestStop();
            return;
        }

        if (glfwGetKey(m_glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            m_app->GetEngine().RequestStop();
            return;
        }

        // Render.
        Render();
    }

    void FixedUpdate(f64 /*fixedDeltaTime*/) override {}
    void LateUpdate(f64 /*deltaTime*/) override {}

private:
    void Render()
    {
        // Animated clear color that shifts over time.
        float t = static_cast<float>(m_elapsed);
        float r = 0.1f + 0.05f * sinf(t * 0.5f);
        float g = 0.1f + 0.05f * sinf(t * 0.3f + 2.0f);
        float b = 0.15f + 0.05f * sinf(t * 0.7f + 4.0f);

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the triangle.
        glUseProgram(m_Program);

        // Set the time uniform for animation.
        GLint timeLoc = glGetUniformLocation(m_Program, "u_Time");
        if (timeLoc >= 0)
            glUniform1f(timeLoc, t);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Swap buffers.
        glfwSwapBuffers(m_glfwWindow);
    }

    bool CompileShaders()
    {
        const char* vertexSrc = R"GLSL(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Color;
            out vec3 v_Color;
            uniform float u_Time;
            void main()
            {
                // Slight rotation animation.
                float c = cos(u_Time * 0.5);
                float s = sin(u_Time * 0.5);
                mat2 rot = mat2(c, -s, s, c);
                vec2 pos = rot * a_Position.xy;
                gl_Position = vec4(pos, a_Position.z, 1.0);
                v_Color = a_Color;
            }
        )GLSL";

        const char* fragmentSrc = R"GLSL(
            #version 330 core
            in vec3 v_Color;
            out vec4 FragColor;
            uniform float u_Time;
            void main()
            {
                // Pulse the color brightness.
                float pulse = 0.8 + 0.2 * sin(u_Time * 2.0);
                FragColor = vec4(v_Color * pulse, 1.0);
            }
        )GLSL";

        // Compile vertex shader.
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexSrc, nullptr);
        glCompileShader(vs);
        if (!CheckShader(vs, "VERTEX"))
        {
            glDeleteShader(vs);
            return false;
        }

        // Compile fragment shader.
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentSrc, nullptr);
        glCompileShader(fs);
        if (!CheckShader(fs, "FRAGMENT"))
        {
            glDeleteShader(vs);
            glDeleteShader(fs);
            return false;
        }

        // Link program.
        m_Program = glCreateProgram();
        glAttachShader(m_Program, vs);
        glAttachShader(m_Program, fs);
        glLinkProgram(m_Program);

        GLint success = GL_FALSE;
        glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
        if (success != GL_TRUE)
        {
            char log[512];
            glGetProgramInfoLog(m_Program, sizeof(log), nullptr, log);
            ENGINE_LOG_ERROR("[Rendering] Program link failed:\n{}", log);
            glDeleteShader(vs);
            glDeleteShader(fs);
            return false;
        }

        glDeleteShader(vs);
        glDeleteShader(fs);
        return true;
    }

    bool CheckShader(GLuint shader, const char* name)
    {
        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE)
        {
            char log[512];
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            ENGINE_LOG_ERROR("[Rendering] {} shader compile failed:\n{}", name, log);
            return false;
        }
        return true;
    }

    void CreateTriangle()
    {
        // Position (xyz) + Color (rgb) per vertex.
        float vertices[] = {
            // Position         // Color
             0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top — red
            -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom-left — green
             0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right — blue
        };

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);

        // Color attribute.
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void*>(3 * sizeof(float)));

        glBindVertexArray(0);
    }

    application::Application* m_app{nullptr};
    GLFWwindow*               m_glfwWindow{nullptr};
    GLuint                    m_Program{0};
    GLuint                    m_VAO{0};
    GLuint                    m_VBO{0};

    u64                       m_frameCount{0};
    f64                       m_elapsed{0.0};
};

// ========================================================================
// Main entry point
// ========================================================================

using engine::core::i32;

int main(i32 argc, const char* argv[])
{
    // Report platform information.
    auto platformInfo = engine::platform::PlatformInfo::Gather();
    std::printf("=== Eclipse Engine Sandbox ===\n");
    std::printf("Platform: %s\n", platformInfo.GetEnginePlatformString().c_str());
    std::printf("Architecture: %s, 64-bit: %s\n\n",
                platformInfo.Architecture.c_str(),
                platformInfo.Is64BitPlatform ? "yes" : "no");

    // Create application.
    application::Application app(argc, argv);

    // Override config for a visible demo.
    auto& config = const_cast<application::ApplicationConfig&>(app.GetConfig());
    config.engineConfig.targetFrameTime = 1.0 / 60.0;
    config.engineConfig.printSubsystemInfo = true;

    // Create and register the rendering subsystem.
    auto renderSubsystem = std::make_unique<RenderingSubsystem>(&app);
    app.GetEngine().GetSubsystemManager().Register(std::move(renderSubsystem));

    // Initialize (creates window + platform).
    if (!app.Initialize())
    {
        std::printf("Sandbox: initialization failed\n");
        return 1;
    }

    // Report window state.
    auto* window = app.GetWindow();
    if (window)
    {
        std::printf("Window created: %s (%ux%u)\n",
                    window->GetTitle().c_str(),
                    window->GetWidth(),
                    window->GetHeight());
    }

    std::printf("\nRendering a colored triangle. Close the window or press ESC to exit.\n\n");

    // Run the main loop — continues until the window is closed or ESC is pressed.
    const i32 exitCode = app.Run();
    std::printf("Sandbox: exited with code %d\n", exitCode);

    return exitCode;
}
