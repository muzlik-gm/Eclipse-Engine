// ============================================================================
// File: Engine/Source/Application/Application.cpp
// Implementation of the Application class — command-line parsing,
// configuration building, platform/window lifecycle, and event processing.
// ============================================================================

#include "Engine/Application/Application.h"
#include "Engine/Core/Log.h"
#include "Engine/Configuration/Config.h"
#include "Engine/Platform/Window.h"
#include "Engine/Platform/PlatformManager.h"
#include "Engine/Events/Event.h"

namespace engine::application
{

    using engine::core::f64;
    using engine::core::i32;
    using engine::core::u32;
    using engine::config::Config;
    using engine::platform::IWindow;
    using engine::platform::WindowProperties;
    using engine::platform::PlatformManager;
    using engine::events::WindowCloseEvent;

    // ========================================================================
    // Construction
    // ========================================================================

    Application::Application(const ApplicationSpec& spec)
        : m_spec(spec)
        , m_commandLine(0, nullptr)
        , m_engine(std::make_unique<Engine>())
    {
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
        if (argc > 0 && argv != nullptr && argv[0] != nullptr)
        {
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
            ENGINE_LOG_WARN("Application \u2014 already initialized");
            return true;
        }

        // Initialize the logging system.
        core::Log::Initialize(m_spec.name);

        const u32 level = m_config.engineConfig.logLevel;
        if (level <= 5)
        {
            core::Log::SetLevel(static_cast<core::LogLevel>(level));
        }

        ENGINE_LOG_INFO("Application '{}' starting", m_spec.name);

        // Initialize the platform subsystem (GLFW).
        auto& platformMgr = PlatformManager::Instance();
        platformMgr.Initialize();
        if (!platformMgr.IsInitialized())
        {
            ENGINE_LOG_ERROR("Application \u2014 platform initialization failed");
            core::Log::Shutdown();
            return false;
        }

        // Create the window unless in headless mode.
        if (!m_config.headless)
        {
            CreateWindow();
        }

        // Create the engine if not already created.
        if (!m_engine)
        {
            m_engine = std::make_unique<Engine>();
        }

        // Initialize the engine.
        if (!m_engine->Initialize(m_config.engineConfig))
        {
            ENGINE_LOG_ERROR("Application \u2014 engine initialization failed");
            m_window.reset();
            platformMgr.Shutdown();
            m_engine.reset();
            core::Log::Shutdown();
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
            ENGINE_LOG_ERROR("Application \u2014 Run() called before successful Initialize()");
            return 1;
        }

        ENGINE_LOG_INFO("Application \u2014 entering main loop");

        // Set up window close callback to stop the engine.
        if (m_window)
        {
            m_window->SetEventCallback([this](engine::events::Event& event)
            {
                if (event.GetEventType() == engine::events::EventType::WindowClose)
                {
                    ENGINE_LOG_INFO("Application \u2014 window close requested");
                    m_engine->RequestStop();
                }
            });
        }

        // In headless mode with no subsystems, run one frame then stop.
        if (!m_window)
        {
            const auto& subsystemMgr = m_engine->GetContext().GetSubsystemManager();
            if (subsystemMgr.Count() == 0)
            {
                ENGINE_LOG_INFO("Application \u2014 headless, no subsystems \u2014 running one demonstration frame");
                m_engine->GetFrameStats().Tick();
                m_engine->GetFrameStats().EndFrame();
                m_engine->RequestStop();
            }
        }

        const i32 exitCode = m_engine->Run();

        ENGINE_LOG_INFO("Application \u2014 main loop exited with code {}", exitCode);
        return exitCode;
    }

    void Application::Shutdown()
    {
        if (!m_initialized)
        {
            return;
        }

        ENGINE_LOG_INFO("Application \u2014 shutting down");

        // Destroy the window first.
        if (m_window)
        {
            m_window->Destroy();
            m_window.reset();
        }

        // Shut down the engine.
        if (m_engine)
        {
            m_engine->Shutdown();
            m_engine.reset();
        }

        // Shut down the platform subsystem.
        PlatformManager::Instance().Shutdown();

        m_initialized = false;

        ENGINE_LOG_INFO("Application \u2014 shutdown complete");

        // Shut down the logging system last.
        core::Log::Shutdown();
    }

    // ========================================================================
    // Accessors
    // ========================================================================

    IWindow* Application::GetWindow() const noexcept
    {
        return m_window.get();
    }

    PlatformManager& Application::GetPlatformManager() noexcept
    {
        return PlatformManager::Instance();
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
            m_config.headless = m_commandLine.GetBool("headless", false);
        }

        if (m_commandLine.Has("app-config"))
        {
            m_config.appConfigFilePath = m_commandLine.Get("app-config");
        }

        ENGINE_LOG_DEBUG("Application \u2014 command line parsed ({} args)",
                         m_commandLine.Args().size());
    }

    void Application::BuildConfig()
    {
        if (!m_config.appConfigFilePath.empty())
        {
            Config appConfig;
            if (appConfig.Load(m_config.appConfigFilePath))
            {
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

                ENGINE_LOG_DEBUG("Application \u2014 loaded app config from '{}'",
                                 m_config.appConfigFilePath);
            }
            else
            {
                ENGINE_LOG_WARN("Application \u2014 failed to load app config file '{}'",
                                m_config.appConfigFilePath);
            }
        }
    }

    void Application::CreateWindow()
    {
        auto& platformMgr = PlatformManager::Instance();

        WindowProperties windowProps;
        windowProps.Title = m_spec.name;
        windowProps.Width  = 1280;
        windowProps.Height = 720;
        windowProps.VisibleOnCreate = true;

        // Apply window-related config overrides.
        if (m_commandLine.Has("width"))
        {
            windowProps.Width = static_cast<u32>(
                m_commandLine.GetInt("width", static_cast<i32>(windowProps.Width)));
        }
        if (m_commandLine.Has("height"))
        {
            windowProps.Height = static_cast<u32>(
                m_commandLine.GetInt("height", static_cast<i32>(windowProps.Height)));
        }
        if (m_commandLine.Has("title"))
        {
            windowProps.Title = m_commandLine.Get("title");
        }

        m_window = platformMgr.CreateWindow(windowProps);
        if (!m_window)
        {
            ENGINE_LOG_ERROR("Application \u2014 failed to create window");
            return;
        }

        ENGINE_LOG_INFO("Application \u2014 window created ({}x{})",
                        windowProps.Width, windowProps.Height);
    }

} // namespace engine::application