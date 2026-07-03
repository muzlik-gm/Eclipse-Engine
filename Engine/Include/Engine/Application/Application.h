// ============================================================================
// File: Engine/Include/Engine/Application/Application.h
// Application class — owns the Engine and Window, parses command line,
// processes platform events, and runs the main entry loop.
// ============================================================================

#pragma once

#include "Engine/Application/ApplicationConfig.h"
#include "Engine/Application/ApplicationSpec.h"
#include "Engine/Core/Types.h"
#include "Engine/Runtime/Engine.h"
#include "Engine/Utilities/CommandLine.h"

#include <memory>
#include <string>
#include <string_view>

namespace engine::platform
{
    class IWindow;
    class PlatformManager;
} // namespace engine::platform

namespace engine::application
{

    using engine::core::i32;
    using engine::runtime::Engine;
    using engine::util::CommandLine;

    // ========================================================================
    // Application
    // ========================================================================

    /// The application layer.
    ///
    /// Ownership hierarchy:
    ///   Application → Engine → ModuleManager
    ///                             → SubsystemManager
    ///                             → EngineContext
    ///                             → FrameStats
    ///   Application → PlatformManager
    ///   Application → Window (via PlatformManager)
    ///
    /// The Application is responsible for:
    ///   - Parsing command-line arguments.
    ///   - Constructing the engine configuration.
    ///   - Initializing the platform subsystem (GLFW).
    ///   - Creating and owning the Window.
    ///   - Processing platform events each frame.
    ///   - Creating and initializing the Engine.
    ///   - Running the main loop (delegated to Engine::Run()).
    ///   - Shutting down cleanly (window, platform, engine).
    ///
    /// Users create an Application via the EntryPoint macro or by
    /// instantiating this class directly.
    class Application
    {
    public:
        /// Constructs an Application from a specification.
        /// Does NOT initialize the engine — call Initialize() then Run().
        explicit Application(const ApplicationSpec& spec);

        /// Constructs an Application from argc/argv.
        /// Convenience overload that builds an ApplicationSpec.
        Application(i32 argc, const char* const argv[]);

        ~Application();

        Application(const Application&)            = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&)                 = delete;
        Application& operator=(Application&&)      = delete;

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------

        /// Initializes the platform, window, and engine.
        /// Returns true on success.
        bool Initialize();

        /// Runs the main loop.  Blocks until the engine stops.
        /// Returns the exit code.
        i32 Run();

        /// Shuts down the engine, window, and application.
        void Shutdown();

        // ----------------------------------------------------------------
        // Accessors
        // ----------------------------------------------------------------

        /// Access the underlying Engine.
        [[nodiscard]] Engine& GetEngine() noexcept { return *m_engine; }

        /// Access the underlying Engine (const).
        [[nodiscard]] const Engine& GetEngine() const noexcept { return *m_engine; }

        /// Access the application specification.
        [[nodiscard]] const ApplicationSpec& GetSpec() const noexcept
        {
            return m_spec;
        }

        /// Access the parsed command line.
        [[nodiscard]] const CommandLine& GetCommandLine() const noexcept
        {
            return m_commandLine;
        }

        /// Access the application configuration.
        [[nodiscard]] ApplicationConfig& GetConfig() noexcept
        {
            return m_config;
        }

        /// Access the application configuration (const).
        [[nodiscard]] const ApplicationConfig& GetConfig() const noexcept
        {
            return m_config;
        }

        /// Access the platform window.  Returns nullptr before Initialize()
        /// or in headless mode.
        [[nodiscard]] engine::platform::IWindow* GetWindow() const noexcept;

        /// Access the platform manager.
        [[nodiscard]] engine::platform::PlatformManager& GetPlatformManager() noexcept;

        // ----------------------------------------------------------------
        // State
        // ----------------------------------------------------------------

        /// Returns true after Initialize() has succeeded.
        [[nodiscard]] bool IsInitialized() const noexcept
        {
            return m_initialized;
        }

        /// Returns true while the main loop is running.
        [[nodiscard]] bool IsRunning() const noexcept;

    private:
        // ----------------------------------------------------------------
        // Internal
        // ----------------------------------------------------------------

        /// Parses command-line arguments and updates the configuration.
        void ParseCommandLine();

        /// Builds the final ApplicationConfig from the spec and
        /// command-line overrides.
        void BuildConfig();

        /// Creates the window if not in headless mode.
        void CreateWindow();

        // ----------------------------------------------------------------
        // Data
        // ----------------------------------------------------------------

        ApplicationSpec                      m_spec;
        ApplicationConfig                    m_config;
        CommandLine                          m_commandLine;
        std::unique_ptr<Engine>               m_engine;
        std::unique_ptr<engine::platform::IWindow> m_window;
        bool                                 m_initialized = false;
    };

} // namespace engine::application