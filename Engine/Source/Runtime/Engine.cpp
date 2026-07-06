// ============================================================================
// File: Engine/Source/Runtime/Engine.cpp
// Implementation of the core Engine class — lifecycle, main loop,
// diagnostics reporting.
// ============================================================================

#include "Engine/Runtime/Engine.h"
#include "Engine/Core/Log.h"
#include "Engine/Diagnostics/BuildInfo.h"
#include "Engine/Configuration/Config.h"
#include "Engine/World/WorldManager.h"

#include <chrono>
#include <thread>
#include <memory>

namespace engine::runtime
{

    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    Engine::Engine()
    {
        ENGINE_LOG_INFO("Engine — constructed (v{})", engine::EngineVersion::String());
    }

    Engine::~Engine()
    {
        if (m_initialized || m_context.GetState() != EngineState::Shutdown)
        {
            ENGINE_LOG_WARN("Engine — destructor called without explicit shutdown, shutting down now");
            Shutdown();
        }
    }

    // ========================================================================
    // Lifecycle
    // ========================================================================

    bool Engine::Initialize(const EngineConfig& config)
    {
        if (m_initialized)
        {
            ENGINE_LOG_WARN("Engine — already initialized, ignoring duplicate call");
            return true;
        }

        m_config = config;
        m_context.SetState(EngineState::Starting);

        // Transition to Initializing.
        if (!TransitionTo(EngineState::Initializing))
        {
            ENGINE_LOG_ERROR("Engine — failed to transition from Starting to Initializing");
            return false;
        }

        // Apply configuration.
        ApplyConfig(config);

        // Load external configuration file if specified.
        LoadConfigFile(config);

        // Print diagnostics.
        if (config.printBuildInfo)
        {
            ReportDiagnostics();
        }

        // Initialize modules first (they may register subsystems).
        ENGINE_LOG_INFO("Engine — initializing modules");
        if (!m_moduleManager.InitializeAll())
        {
            ENGINE_LOG_ERROR("Engine — module initialization failed");
            TransitionTo(EngineState::Stopping);
            Shutdown();
            return false;
        }

        // Register the WorldManager subsystem and bind the EventBus so
        // that world / scene / entity / component events are dispatched.
        {
            auto worldManager = std::make_unique<world::WorldManager>();
            worldManager->SetEventBus(&m_context.GetEventBus());
            m_context.GetSubsystemManager().Register(std::move(worldManager));
            ENGINE_LOG_INFO("Engine — registered WorldManager subsystem");
        }

        // Initialize subsystems.
        ENGINE_LOG_INFO("Engine — initializing subsystems");
        if (!m_context.GetSubsystemManager().InitializeAll())
        {
            ENGINE_LOG_ERROR("Engine — subsystem initialization failed");
            TransitionTo(EngineState::Stopping);
            Shutdown();
            return false;
        }

        // Print subsystem info if requested.
        if (config.printSubsystemInfo)
        {
            const auto names = m_context.GetSubsystemManager().GetNames();
            ENGINE_LOG_INFO("Engine — {} subsystem(s) registered:", names.size());
            for (const auto& n : names)
            {
                ENGINE_LOG_INFO("  - {}", n);
            }
        }

        // Transition to Running.
        if (!TransitionTo(EngineState::Running))
        {
            ENGINE_LOG_ERROR("Engine — failed to transition to Running");
            Shutdown();
            return false;
        }

        m_initialized = true;
        ENGINE_LOG_INFO("Engine — initialization complete");

        // Reset frame stats for the main loop.
        m_context.GetFrameStats().Reset();

        return true;
    }

    void Engine::Shutdown()
    {
        const EngineState currentState = m_context.GetState();
        if (currentState == EngineState::Shutdown)
        {
            ENGINE_LOG_DEBUG("Engine — already shut down, ignoring duplicate call");
            return;
        }

        // Transition to Stopping (unless already there).
        if (currentState != EngineState::Stopping)
        {
            if (!TransitionTo(EngineState::Stopping))
            {
                ENGINE_LOG_WARN("Engine — illegal transition to Stopping from state {}",
                                EngineStateToString(currentState));
            }
        }

        // Stop the main loop if running.
        m_running = false;

        // Shut down subsystems (reverse order).
        ENGINE_LOG_INFO("Engine — shutting down subsystems");
        m_context.GetSubsystemManager().ShutdownAll();

        // Shut down modules (reverse order).
        ENGINE_LOG_INFO("Engine — shutting down modules");
        m_moduleManager.ShutdownAll();

        // Transition to Shutdown.
        TransitionTo(EngineState::Shutdown);

        m_initialized = false;
        ENGINE_LOG_INFO("Engine — shutdown complete");
    }

    // ========================================================================
    // Main loop
    // ========================================================================

    i32 Engine::Run()
    {
        if (!m_initialized)
        {
            ENGINE_LOG_ERROR("Engine — Run() called before Initialize()");
            return 1;
        }

        if (m_context.GetState() != EngineState::Running)
        {
            ENGINE_LOG_ERROR("Engine — Run() called in state {} (expected Running)",
                             EngineStateToString(m_context.GetState()));
            return 1;
        }

        ENGINE_LOG_INFO("Engine — main loop started");
        m_running = true;

        while (m_running)
        {
            ProcessFrame();
        }

        ENGINE_LOG_INFO("Engine — main loop exited ({} frames)",
                        m_context.GetFrameStats().FrameCount());

        return 0;
    }

    void Engine::RequestStop()
    {
        ENGINE_LOG_INFO("Engine — stop requested");
        m_running = false;
    }

    // ========================================================================
    // State
    // ========================================================================

    EngineState Engine::GetState() const noexcept
    {
        return m_context.GetState();
    }

    bool Engine::IsRunning() const noexcept
    {
        return m_context.GetState() == EngineState::Running;
    }

    // ========================================================================
    // Reporting
    // ========================================================================

    void Engine::ReportDiagnostics() const
    {
        ENGINE_LOG_INFO("{}", diagnostics::BuildInfo::GetFullBuildInfoString());
        ENGINE_LOG_INFO("Engine v{}", engine::EngineVersion::String());
    }

    // ========================================================================
    // Internal
    // ========================================================================

    void Engine::ProcessFrame()
    {
        FrameStats& stats = m_context.GetFrameStats();

        // Record frame start for frame-rate limiting.
        const core::TimePoint frameStart = core::Clock::Now();

        // Advance timing.
        stats.Tick();

        const f64 dt = stats.DeltaTime();
        const f64 fixedDt = stats.FixedDeltaTime();

        // Handle pause state.
        const EngineState state = m_context.GetState();
        if (state == EngineState::Paused)
        {
            // Fixed update still runs when paused for physics stability.
            usize pausedFixedSteps = 0;
            while (stats.ShouldFixedUpdate() && pausedFixedSteps < stats.MaxFixedStepsPerFrame())
            {
                m_context.GetSubsystemManager().FixedUpdateAll(fixedDt);
                stats.ConsumeFixedStep();
                ++pausedFixedSteps;
            }

            stats.EndFrame();
            return;
        }

        // Fixed update loop.
        usize fixedSteps = 0;
        while (stats.ShouldFixedUpdate() && fixedSteps < stats.MaxFixedStepsPerFrame())
        {
            m_context.GetSubsystemManager().FixedUpdateAll(fixedDt);
            stats.ConsumeFixedStep();
            ++fixedSteps;
        }

        // Variable update.
        m_context.GetSubsystemManager().UpdateAll(dt);

        // Late update.
        m_context.GetSubsystemManager().LateUpdateAll(dt);

        // End frame.
        stats.EndFrame();

        // Frame-rate limiting: sleep for the remainder of the target frame time.
        const f64 targetFt = stats.TargetFrameTime();
        if (targetFt > 0.0)
        {
            const f64 frameElapsed = core::Clock::SecondsSince(frameStart);
            const f64 remaining = targetFt - frameElapsed;
            if (remaining > 0.0)
            {
                std::this_thread::sleep_for(
                    std::chrono::duration<f64>(remaining));
            }
        }
    }

    void Engine::ApplyConfig(const EngineConfig& config)
    {
        // Store config in context.
        m_context.GetConfig() = config;

        // Apply to frame stats.
        FrameStats& stats = m_context.GetFrameStats();
        stats.SetFixedDeltaTime(config.fixedDeltaTime);
        stats.SetMaxFixedStepsPerFrame(config.maxFixedStepsPerFrame);
        stats.SetTargetFrameTime(config.targetFrameTime);
        stats.SetTimeScale(config.timeScale);

        ENGINE_LOG_DEBUG("Engine — config applied: fixedDt={:.6f}s, maxFixedSteps={}, targetFrameTime={:.6f}s, timeScale={:.2f}",
                         config.fixedDeltaTime,
                         config.maxFixedStepsPerFrame,
                         config.targetFrameTime,
                         config.timeScale);
    }

    void Engine::LoadConfigFile(const EngineConfig& config)
    {
        if (config.configFilePath.empty())
        {
            return;
        }

        config::Config fileConfig;
        if (!fileConfig.Load(config.configFilePath))
        {
            ENGINE_LOG_WARN("Engine — failed to load config file '{}'", config.configFilePath);
            return;
        }

        // Override values from the config file.
        if (fileConfig.Has("engine.fixedDeltaTime"))
        {
            const f64 val = fileConfig.GetFloat("engine.fixedDeltaTime",
                                                config.fixedDeltaTime);
            if (val > 0.0)
            {
                m_config.fixedDeltaTime = val;
            }
        }

        if (fileConfig.Has("engine.maxFixedStepsPerFrame"))
        {
            const usize val = static_cast<usize>(
                fileConfig.GetInt("engine.maxFixedStepsPerFrame",
                                  static_cast<core::i64>(config.maxFixedStepsPerFrame)));
            if (val >= 1)
            {
                m_config.maxFixedStepsPerFrame = val;
            }
        }

        if (fileConfig.Has("engine.targetFrameTime"))
        {
            m_config.targetFrameTime = fileConfig.GetFloat(
                "engine.targetFrameTime", config.targetFrameTime);
        }

        if (fileConfig.Has("engine.timeScale"))
        {
            m_config.timeScale = fileConfig.GetFloat(
                "engine.timeScale", config.timeScale);
        }

        if (fileConfig.Has("engine.logLevel"))
        {
            m_config.logLevel = static_cast<u32>(
                fileConfig.GetInt("engine.logLevel",
                                  static_cast<core::i64>(config.logLevel)));
        }

        if (fileConfig.Has("engine.printBuildInfo"))
        {
            m_config.printBuildInfo = fileConfig.GetBool(
                "engine.printBuildInfo", config.printBuildInfo);
        }

        // Re-apply the updated config.
        ApplyConfig(m_config);

        ENGINE_LOG_INFO("Engine — loaded configuration from '{}'", config.configFilePath);
    }

    bool Engine::TransitionTo(EngineState newState)
    {
        const EngineState current = m_context.GetState();

        if (!IsEngineStateTransitionValid(current, newState))
        {
            ENGINE_LOG_ERROR("Engine — illegal state transition: {} -> {}",
                             EngineStateToString(current),
                             EngineStateToString(newState));
            return false;
        }

        m_context.SetState(newState);
        ENGINE_LOG_DEBUG("Engine — state: {} -> {}",
                         EngineStateToString(current),
                         EngineStateToString(newState));
        return true;
    }

} // namespace engine::runtime