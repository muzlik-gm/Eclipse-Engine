// ============================================================================
// File: Sandbox/Source/SandboxMain.cpp
// Minimal sandbox executable that exercises the Phase 2 platform layer
// — Application, Engine, Window, and the main loop.
// ============================================================================

#include "Engine/Core/Types.h"
#include "Engine/Core/Log.h"
#include "Engine/Application/Application.h"
#include "Engine/Runtime/Engine.h"
#include "Engine/Runtime/ISubsystem.h"
#include "Engine/Platform/Window.h"
#include "Engine/Platform/PlatformInfo.h"

#include <chrono>
#include <cstdio>
#include <functional>
#include <string>
#include <thread>

using namespace engine;
using engine::core::f64;
using engine::core::u64;

/// A trivial subsystem that logs its lifecycle calls and auto-stops
/// the engine after a configured duration.
class DemoSubsystem final : public runtime::ISubsystem
{
public:
    explicit DemoSubsystem(std::string name, f64 runDurationSeconds)
        : m_name(std::move(name))
        , m_runDuration(runDurationSeconds)
    {}

    [[nodiscard]] std::string_view GetName() const noexcept override
    {
        return m_name;
    }

    bool Initialize() override
    {
        ENGINE_LOG_INFO("[{}] Initialize()", m_name);
        m_updateCount = 0;
        return true;
    }

    void Shutdown() override
    {
        ENGINE_LOG_INFO("[{}] Shutdown() — {} updates, {} fixed updates",
                         m_name, m_updateCount, m_fixedUpdateCount);
    }

    void Update(f64 deltaTime) override
    {
        ++m_updateCount;
        m_elapsed += deltaTime;

        if (m_elapsed >= m_runDuration && m_stopCallback)
        {
            ENGINE_LOG_INFO("[{}] elapsed {:.3f}s >= {:.3f}s — requesting engine stop",
                             m_name, m_elapsed, m_runDuration);
            m_stopCallback();
        }
    }

    void FixedUpdate(f64 /*fixedDeltaTime*/) override
    {
        ++m_fixedUpdateCount;
    }

    void LateUpdate(f64 /*deltaTime*/) override
    {
        // Intentionally empty for the demo.
    }

    void SetStopCallback(std::function<void()> callback)
    {
        m_stopCallback = std::move(callback);
    }

private:
    std::string              m_name;
    f64                      m_runDuration       = 1.0;
    f64                      m_elapsed           = 0.0;
    u64                      m_updateCount       = 0;
    u64                      m_fixedUpdateCount  = 0;
    std::function<void()>    m_stopCallback;
};

using engine::core::i32;

int main(i32 argc, const char* argv[])
{
    // Report platform information before creating the application.
    auto platformInfo = engine::platform::PlatformInfo::Gather();
    std::printf("Platform: %s\n", platformInfo.GetEnginePlatformString().c_str());
    std::printf("Architecture: %s, 64-bit: %s\n\n",
                platformInfo.Architecture.c_str(),
                platformInfo.Is64BitPlatform ? "yes" : "no");

    // Create application.
    application::Application app(argc, argv);

    // Create and register a demo subsystem that runs for ~2 seconds.
    auto demoSubsystem = std::make_unique<DemoSubsystem>("Demo", 2.0);
    DemoSubsystem* rawDemo = demoSubsystem.get();
    rawDemo->SetStopCallback([&app]() { app.GetEngine().RequestStop(); });

    app.GetEngine().GetSubsystemManager().Register(std::move(demoSubsystem));

    // Override config for a visible demo.
    auto& config = const_cast<application::ApplicationConfig&>(app.GetConfig());
    config.engineConfig.targetFrameTime = 1.0 / 60.0;
    config.engineConfig.printSubsystemInfo = true;

    // Initialize (creates window and platform).
    if (!app.Initialize())
    {
        std::printf("Sandbox: initialization failed\n");
        std::printf("Tip: run with --headless to skip window creation\n");
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

    // Run.
    const i32 exitCode = app.Run();
    std::printf("Sandbox: exited with code %d\n", exitCode);

    return exitCode;
}