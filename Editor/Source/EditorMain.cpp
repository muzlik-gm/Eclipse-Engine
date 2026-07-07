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

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

// Force-link the OpenGL backend so its static self-registration runs.
// Without this, the linker strips OpenGLBackend.cpp from the static
// Engine library and the backend is never registered.
extern "C" void ForceLinkOpenGLBackend();

using namespace engine;
using engine::core::i32;
using engine::core::u32;
using engine::core::u64;
using engine::core::f64;

#ifdef _WIN32
static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep)
{
    // Disable any previous filter so infinite crashes don't loop.
    SetUnhandledExceptionFilter(nullptr);

    DWORD code = ep->ExceptionRecord->ExceptionCode;
    void* addr = ep->ExceptionRecord->ExceptionAddress;

    ENGINE_LOG_CRITICAL("=== CRASH 0x{:08X} at {:p} ===", static_cast<u32>(code), addr);

    // Attempt a minimal stack walk (up to 16 frames).
    HANDLE proc = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    SymInitialize(proc, nullptr, TRUE);

    CONTEXT ctx = *ep->ContextRecord;
    STACKFRAME64 frame = {};
    frame.AddrPC.Offset = ctx.Rip;
    frame.AddrFrame.Offset = ctx.Rbp;
    frame.AddrStack.Offset = ctx.Rsp;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

#ifdef _M_X64
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
#else
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
#endif

    for (int i = 0; i < 16; ++i)
    {
        if (!StackWalk64(machineType, proc, thread, &frame, &ctx,
                         nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
            break;

        if (frame.AddrPC.Offset == 0)
            break;

        // Try to resolve symbol name.
        DWORD64 displacement = 0;
        char buf[sizeof(SYMBOL_INFO) + 256] = {};
        auto* sym = reinterpret_cast<SYMBOL_INFO*>(buf);
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 255;

        if (SymFromAddr(proc, frame.AddrPC.Offset, &displacement, sym))
            ENGINE_LOG_CRITICAL("  {:2d} {}+0x{:X}", i, sym->Name, displacement);
        else
            ENGINE_LOG_CRITICAL("  {:2d} {:p}", i, reinterpret_cast<void*>(frame.AddrPC.Offset));
    }

    ENGINE_LOG_CRITICAL("=== End crash dump ===");
    core::Log::Shutdown();

    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(i32 argc, const char* argv[])
{
    // Initialize logging.
    core::Log::Initialize("EclipseEditor");

#ifdef _WIN32
    SetUnhandledExceptionFilter(CrashHandler);
#endif

    ENGINE_LOG_INFO("=== Eclipse Engine Editor ===");

    // Force the OpenGL backend to be registered before anything else.
    ForceLinkOpenGLBackend();

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

    // Make the OpenGL context current BEFORE initializing GLAD or ImGui.
    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1); // Enable vsync.

    // Initialize GLAD.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        ENGINE_LOG_ERROR("Failed to initialize GLAD");
        return 1;
    }

    ENGINE_LOG_INFO("OpenGL {} — {}",
                    reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                    reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

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
        ENGINE_LOG_ERROR("Failed to initialize renderer — continuing without it");
        // Don't return — we still want ImGui to work for the editor UI.
    }

    // Create and initialize the editor.
    editor::EditorApplication editorApp;
    if (!editorApp.Initialize(glfwWindow, &engineRef.GetContext().GetEventBus(),
                               activeScene, renderer.get()))
    {
        ENGINE_LOG_ERROR("Failed to initialize editor");
        return 1;
    }

    ENGINE_LOG_INFO("Editor ready — entering main loop");

    // Main loop.
    u64 frameCount = 0;
    while (!glfwWindowShouldClose(glfwWindow))
    {
        ++frameCount;

        glfwPollEvents();

        // Calculate delta time.
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        f64 deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        editorApp.GetContext().SetDeltaTime(deltaTime);

        // Log frame count every 500 frames for crash debugging.
        if (frameCount % 500 == 0)
            ENGINE_LOG_INFO("MainLoop — frame {}", frameCount);

        // Periodic GL error check (every 300 frames).
        if (frameCount % 300 == 0)
        {
            GLenum err = glGetError();
            while (err != GL_NO_ERROR)
            {
                ENGINE_LOG_ERROR("MainLoop — GL error 0x{:X} at frame {}", err, frameCount);
                err = glGetError();
            }
        }

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
