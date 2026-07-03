// ============================================================================
// File: Engine/Source/Application/Application.cpp
// Implementation of the Application class — command-line parsing,
// configuration building, and lifecycle delegation to the Engine.
// ============================================================================

#include "Engine/Application/Application.h"
#include "Engine/Core/Log.h"
#include "Engine/Configuration/Config.h"

#include <thread>

namespace engine::application
{

    using engine::core::f64;
    using engine::core::i32;
    using engine::core::u32;
    using engine::config::Config;

    // ========================================================================
    // Construction
    // ========================================================================

    Application::Application(const ApplicationSpec& spec)
        : m_spec(spec)
        , m_commandLine(0, nullptr)
        , m_engine(std::make_unique<Engine>())
    {
        // Build argv for CommandLine from spec.commandLineArgs.
        if (!m_spec.commandLineArgs.empty())
        {
            std::vector<const char*> argv;
            argv.reserve(m_spec.commandLineArgs.size());
            for (const auto& arg : m_spec.commandLineArgs)
            {
                argv.push_back(arg.c_str());
            }
            m_commandLine = CommandLine(
                static_cast<i32>(argv.size()),
                argv.data());
        }

        ParseCommandLine();
        BuildConfig();
    }

    Application::Application(i32 argc, const char* const argv[])
        : m_commandLine(argc, argv)
        , m_engine(std::make_unique<Engine>())
    {
        // Derive application name from argv[0].
        if (argc > 0 && argv != nullptr && argv[0] != nullptr)
        {
            // Take just the filename component.
            std::string exeName(argv[0]);
            const auto pos = exeName.find_last_of("/\\");
            if (pos != std::string::npos)
            {
                m_spec.name = exeName.substr(pos + 1);
            }
            else
            {
                m_spec.name = exeName;
            }
        }

        // Store raw args in spec for completeness.
        m_spec.commandLineArgs = m_commandLine.Args();

        ParseCommandLine();
        BuildConfig();
    }

    Application::~Application()
    {
        Shutdown();
    }

    // ========================================================================
    // Lifecycle
    // ========================================================================

    bool Application::Initialize()
    {
        if (m_initialized)
        {
            ENGINE_LOG_WARN("Application — already initialized");
            return true;
        }

        // Initialize the logging system.
        core::Log::Initialize(m_spec.name);

        // Apply log level from config.
        const u32 level = m_config.engineConfig.logLevel;
        if (level <= 5)
        {
            core::Log::SetLevel(static_cast<core::LogLevel>(level));
        }

        ENGINE_LOG_INFO("Application '{}' starting", m_spec.name);

        // Create the engine if not already created.
        if (!m_engine)
        {
            m_engine = std::make_unique<Engine>();
        }

        // Initialize the engine.
        if (!m_engine->Initialize(m_config.engineConfig))
        {
            ENGINE_LOG_ERROR("Application — engine initialization failed");
            m_engine.reset();
            return false;
        }

        m_initialized = true;
        ENGINE_LOG_INFO("Application '{}' initialized", m_spec.name);

        return true;
    }

    i32 Application::Run()
    {
        if (!m_initialized || !m_engine)
        {
            ENGINE_LOG_ERROR("Application — Run() called before successful Initialize()");
            return 1;
        }

        ENGINE_LOG_INFO("Application — entering main loop");

        // In Phase 1 the engine runs until RequestStop() is called.
        // Without a window or input system, we stop after a brief
        // demonstration run when no subsystems are registered.
        //
        // If subsystems are present, they control the lifecycle.
        // If none are present, run for one frame then stop (headless mode).
        const auto& subsystemMgr = m_engine->GetContext().GetSubsystemManager();
        if (subsystemMgr.Count() == 0)
        {
            ENGINE_LOG_INFO("Application — no subsystems registered, running one demonstration frame");
            m_engine->GetFrameStats().Tick();
            m_engine->GetFrameStats().EndFrame();
            m_engine->RequestStop();
        }

        const i32 exitCode = m_engine->Run();

        ENGINE_LOG_INFO("Application — main loop exited with code {}", exitCode);
        return exitCode;
    }

    void Application::Shutdown()
    {
        if (!m_initialized)
        {
            return;
        }

        ENGINE_LOG_INFO("Application — shutting down");

        if (m_engine)
        {
            m_engine->Shutdown();
            m_engine.reset();
        }

        m_initialized = false;

        ENGINE_LOG_INFO("Application — shutdown complete");

        // Shut down the logging system last.
        core::Log::Shutdown();
    }

    // ========================================================================
    // State
    // ========================================================================

    bool Application::IsRunning() const noexcept
    {
        return m_initialized && m_engine && m_engine->IsRunning();
    }

    // ========================================================================
    // Internal
    // ========================================================================

    void Application::ParseCommandLine()
    {
        // Override engine config values from command line.
        if (m_commandLine.Has("config"))
        {
            m_config.engineConfig.configFilePath = m_commandLine.Get("config");
        }

        if (m_commandLine.Has("log-level"))
        {
            m_config.engineConfig.logLevel = static_cast<u32>(
                m_commandLine.GetInt("log-level",
                                     static_cast<i32>(m_config.engineConfig.logLevel)));
        }

        if (m_commandLine.Has("fixed-dt"))
        {
            const f64 val = m_commandLine.GetFloat("fixed-dt");
            if (val > 0.0)
            {
                m_config.engineConfig.fixedDeltaTime = val;
            }
        }

        if (m_commandLine.Has("no-build-info"))
        {
            m_config.engineConfig.printBuildInfo = false;
        }

        if (m_commandLine.Has("headless"))
        {
            m_config.headless = m_commandLine.GetBool("headless", true);
        }

        if (m_commandLine.Has("app-config"))
        {
            m_config.appConfigFilePath = m_commandLine.Get("app-config");
        }

        ENGINE_LOG_DEBUG("Application — command line parsed ({} args)",
                         m_commandLine.Args().size());
    }

    void Application::BuildConfig()
    {
        // Load application config file if specified.
        if (!m_config.appConfigFilePath.empty())
        {
            Config appConfig;
            if (appConfig.Load(m_config.appConfigFilePath))
            {
                // Merge engine-relevant overrides.
                if (appConfig.Has("engine.fixedDeltaTime"))
                {
                    const f64 val = appConfig.GetFloat("engine.fixedDeltaTime");
                    if (val > 0.0)
                    {
                        m_config.engineConfig.fixedDeltaTime = val;
                    }
                }
                if (appConfig.Has("engine.timeScale"))
                {
                    m_config.engineConfig.timeScale = appConfig.GetFloat("engine.timeScale");
                }
                if (appConfig.Has("headless"))
                {
                    m_config.headless = appConfig.GetBool("headless", m_config.headless);
                }

                ENGINE_LOG_DEBUG("Application — loaded app config from '{}'",
                                 m_config.appConfigFilePath);
            }
            else
            {
                ENGINE_LOG_WARN("Application — failed to load app config file '{}'",
                                m_config.appConfigFilePath);
            }
        }
    }

} // namespace engine::application