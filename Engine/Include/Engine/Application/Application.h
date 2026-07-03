// ============================================================================
// File: Engine/Include/Engine/Application/Application.h
// Application class — owns the Engine, parses command line, and runs
// the main entry loop.
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
    ///
    /// The Application is responsible for:
    ///   - Parsing command-line arguments.
    ///   - Constructing the engine configuration.
    ///   - Creating and initializing the Engine.
    ///   - Running the main loop (delegated to Engine::Run()).
    ///   - Shutting down cleanly.
    ///
    /// In Phase 1 there is no window creation, no rendering, and no
    /// platform-specific code.  The Application simply drives the engine
    /// lifecycle.
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

        /// Initializes the engine with the computed configuration.
        /// Returns true on success.
        bool Initialize();

        /// Runs the main loop.  Blocks until the engine stops.
        /// Returns the exit code.
        i32 Run();

        /// Shuts down the engine and the application.
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
        [[nodiscard]] const ApplicationConfig& GetConfig() const noexcept
        {
            return m_config;
        }

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

        // ----------------------------------------------------------------
        // Data
        // ----------------------------------------------------------------

        ApplicationSpec              m_spec;
        ApplicationConfig            m_config;
        CommandLine                  m_commandLine;
        std::unique_ptr<Engine>      m_engine;
        bool                         m_initialized = false;
    };

} // namespace engine::application