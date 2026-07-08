// ============================================================================
// File: Editor/Source/EditorMain.cpp
// Entry point for the Eclipse Engine Editor.
// ============================================================================
#include "Engine/Core/Log.h"
#include "Engine/Core/Types.h"
#include "Engine/Application/Application.h"
#include "Engine/Runtime/Engine.h"
#include "Engine/Platform/Window.h"
#include "Engine/Platform/PlatformManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/World/WorldManager.h"
#include "Engine/Renderer/Core/Renderer.h"
#include "Editor/Core/EditorApplication.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>

// Force-link the OpenGL backend so its static self-registration runs.
extern "C" void ForceLinkOpenGLBackend();

using namespace engine;
using engine::core::i32;
using engine::core::u32;
using engine::core::u64;
using engine::core::f64;

// File-scoped editor app pointer — used instead of glfwSetWindowUserPointer
// to avoid overwriting ImGui's stored user pointer (which would crash all
// GLFW callbacks registered by ImGui, e.g. on window maximize/restore).
static editor::EditorApplication* g_EditorApp = nullptr;

// Helper: print to BOTH stderr and the log file so the user can see
// errors in the terminal even if spdlog doesn't flush.
#define FATAL_ERROR(fmt, ...) \
    do { \
        std::fprintf(stderr, "FATAL: " fmt "\n", ##__VA_ARGS__); \
        std::fflush(stderr); \
        ENGINE_LOG_ERROR(fmt, ##__VA_ARGS__); \
        core::Log::Shutdown(); \
        std::system("pause"); \
        return 1; \
    } while(0)

int main(i32 argc, const char* argv[])
{
    // Initialize logging — this writes to both console and file.
    core::Log::Initialize("EclipseEditor");

    std::fprintf(stderr, "Eclipse Engine Editor starting...\n");
    std::fflush(stderr);

    ENGINE_LOG_INFO("=== Eclipse Engine Editor ===");

    // Force the OpenGL backend to be registered.
    ForceLinkOpenGLBackend();

    // Create the application + engine.
    application::Application app(argc, argv);
    auto& engineRef = app.GetEngine();

    // Initialize the application (creates window + platform).
    if (!app.Initialize())
    {
        FATAL_ERROR("Failed to initialize application.");
    }

    // Get the GLFW window from the application.
    auto* window = app.GetWindow();
    if (!window)
    {
        FATAL_ERROR("Application returned null window. OpenGL context creation may have failed.");
    }

    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->GetNativeHandle());
    if (!glfwWindow)
    {
        FATAL_ERROR("Window has null native handle.");
    }

    std::fprintf(stderr, "Window created: %ux%u\n", window->GetWidth(), window->GetHeight());
    std::fflush(stderr);

    // Make the OpenGL context current BEFORE initializing GLAD or ImGui.
    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1); // Enable vsync.

    // Initialize GLAD.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        FATAL_ERROR("Failed to initialize GLAD. Your GPU driver may not support OpenGL 3.3+.");
    }

    const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    ENGINE_LOG_INFO("OpenGL {} — {}", glVersion, glRenderer);
    std::fprintf(stderr, "OpenGL: %s — %s\n", glVersion, glRenderer);
    std::fflush(stderr);

    // Get the active scene from the WorldManager.
    scene::Scene* activeScene = nullptr;
    auto* worldManager = engineRef.GetSubsystemManager().GetRaw("WorldManager");
    if (worldManager)
    {
        auto* wm = static_cast<world::WorldManager*>(worldManager);
        activeScene = wm->GetWorld().GetActiveScene();
    }

    // Create the renderer.
    renderer::RendererConfiguration rendererConfig;
    rendererConfig.Backend = rhi::GraphicsBackend::OpenGL;
    rendererConfig.WindowHandle = glfwWindow;
    rendererConfig.Width = window->GetWidth();
    rendererConfig.Height = window->GetHeight();
    rendererConfig.EnableValidation = true;

    auto renderer = std::make_unique<renderer::Renderer>();
    renderer->SetConfiguration(rendererConfig);

    if (!renderer->Initialize())
    {
        std::fprintf(stderr, "WARNING: Failed to initialize renderer — continuing without it.\n");
        std::fflush(stderr);
        ENGINE_LOG_ERROR("Failed to initialize renderer — continuing without it");
    }

    // Create and initialize the editor.
    editor::EditorApplication editorApp;
    if (!editorApp.Initialize(glfwWindow, &engineRef.GetContext().GetEventBus(),
                               activeScene, renderer.get()))
    {
        FATAL_ERROR("Failed to initialize editor.");
    }

    // Store editor app pointer for use in C-style callbacks.
    // NOTE: We use a file-scoped global instead of glfwSetWindowUserPointer
    // because ImGui owns the user pointer for its own callbacks (resize, focus,
    // etc.). Overwriting it would crash on window maximize/restore.
    g_EditorApp = &editorApp;

    // Set up keyboard shortcut callback.
    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* win, int key, int scancode, int action, int mods)
    {
        (void)win;
        (void)scancode;
        auto* app = g_EditorApp;
        if (!app) return;
        if (action != GLFW_PRESS && action != GLFW_REPEAT)
            return;
        // Convert GLFW mod bits to KeyModifiers layout (Ctrl=1, Shift=2, Alt=4, Super=8).
        u32 editorMods = 0;
        if (mods & GLFW_MOD_CONTROL) editorMods |= 1u;
        if (mods & GLFW_MOD_SHIFT)   editorMods |= 2u;
        if (mods & GLFW_MOD_ALT)     editorMods |= 4u;
        if (mods & GLFW_MOD_SUPER)   editorMods |= 8u;
        app->ProcessKey(static_cast<u32>(key), editorMods);
    });

    ENGINE_LOG_INFO("Editor ready — entering main loop");
    std::fprintf(stderr, "Editor ready.\n");
    std::fflush(stderr);

    // Main loop.
    while (!glfwWindowShouldClose(glfwWindow))
    {
        glfwPollEvents();

        // Calculate delta time.
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        f64 deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        editorApp.GetContext().SetDeltaTime(deltaTime);

        // Update the engine (runtime simulation).
        engineRef.GetSubsystemManager().UpdateAll(deltaTime);

        // Render the editor.
        editorApp.Render();

        // Swap buffers.
        glfwSwapBuffers(glfwWindow);
    }

    // Shutdown.
    editorApp.Shutdown();
    renderer->Shutdown();
    app.Shutdown();
    core::Log::Shutdown();

    return 0;
}
