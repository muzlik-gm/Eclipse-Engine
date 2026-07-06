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

using namespace engine;
using engine::core::i32;
using engine::core::f64;

int main(i32 argc, const char* argv[])
{
    // Initialize logging.
    core::Log::Initialize("EclipseEditor");

    ENGINE_LOG_INFO("=== Eclipse Engine Editor ===");

    // Create the application + engine.
    application::Application app(argc, argv);
    auto& engineRef = app.GetEngine();

    // Initialize the application (creates window + platform).
    if (!app.Initialize())
    {
        ENGINE_LOG_ERROR("Failed to initialize application");
        return 1;
    }

    // Get the GLFW window from the application.
    auto* window = app.GetWindow();
    GLFWwindow* glfwWindow = nullptr;
    if (window)
    {
        glfwWindow = static_cast<GLFWwindow*>(window->GetNativeHandle());
    }

    if (!glfwWindow)
    {
        ENGINE_LOG_ERROR("Failed to get GLFW window");
        return 1;
    }

    // Get the active scene from the WorldManager.
    scene::Scene* activeScene = nullptr;
    auto* worldManager = engineRef.GetSubsystemManager().GetRaw("WorldManager");
    if (worldManager)
    {
        auto* wm = static_cast<world::WorldManager*>(worldManager);
        activeScene = wm->GetWorld().GetActiveScene();
    }

    // Create the renderer (simplified — in production this would be
    // configured via the RendererConfiguration).
    renderer::RendererConfiguration rendererConfig;
    rendererConfig.Backend = rhi::GraphicsBackend::OpenGL;
    rendererConfig.WindowHandle = glfwWindow;
    rendererConfig.Width = 1280;
    rendererConfig.Height = 720;
    rendererConfig.EnableValidation = true;

    auto renderer = std::make_unique<renderer::Renderer>();
    renderer->SetConfiguration(rendererConfig);
    renderer->Initialize();

    // Create and initialize the editor.
    editor::EditorApplication editorApp;
    if (!editorApp.Initialize(glfwWindow, &engineRef.GetContext().GetEventBus(),
                               activeScene, renderer.get()))
    {
        ENGINE_LOG_ERROR("Failed to initialize editor");
        return 1;
    }

    // Set up key callback for shortcuts.
    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* /*win*/, int key, int /*scancode*/,
                                       int action, int mods)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            // Forward to the editor via a global pointer.
            // In production, this would use a proper event system.
        }
    });

    ENGINE_LOG_INFO("Editor ready — entering main loop");

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
