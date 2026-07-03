// ============================================================================
// File: Engine/Include/Engine/Runtime/Engine.h
// Core engine class — owns all runtime subsystems, manages the lifecycle,
// drives the update loop, and exposes stable public interfaces.
// ============================================================================

#pragma once

#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Version.h"
#include "Engine/Runtime/EngineConfig.h"
#include "Engine/Runtime/EngineContext.h"
#include "Engine/Runtime/EngineState.h"
#include "Engine/Runtime/FrameStats.h"
#include "Engine/Runtime/ModuleManager.h"
#include "Engine/Runtime/SubsystemManager.h"

#include <string>
#include <string_view>

namespace engine::runtime
{

    using engine::core::i32;

    // ========================================================================
    // Engine
    // ========================================================================

    /// The core engine class.
    ///
    /// The Engine owns all runtime subsystems (via SubsystemManager) and
    /// modules (via ModuleManager).  It manages the complete lifecycle from
    /// initialization through the main loop to shutdown.
    ///
    /// Ownership hierarchy:
    ///   Application → Engine → ModuleManager
    ///                             → SubsystemManager
    ///                             → EngineContext
    ///                             → FrameStats
    ///
    /// The Engine exposes only stable public interfaces.  Internal state
    /// is accessed through the EngineContext by subsystems.
    class Engine
    {
    public:
        Engine();
        ~Engine();

        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------

        /// Initializes the engine with the given configuration.
        /// Call this once before Run().
        ///
        /// Steps:
        ///   1. Transition to Initializing.
        ///   2. Apply configuration (timing, logging, etc.).
        ///   3. Load engine config file (if specified).
        ///   4. Initialize modules in dependency order.
        ///   5. Initialize subsystems in dependency order.
        ///   6. Transition to Running.
        ///
        /// Returns true if all initializations succeeded.
        bool Initialize(const EngineConfig& config);

        /// Shuts down the engine.  Called automatically by the destructor
        /// if not called manually.
        ///
        /// Steps:
        ///   1. Transition to Stopping.
        ///   2. Shut down subsystems in reverse order.
        ///   3. Shut down modules in reverse order.
        ///   4. Transition to Shutdown.
        void Shutdown();

        // ----------------------------------------------------------------
        // Main loop
        // ----------------------------------------------------------------

        /// Runs the main loop.  Blocks until the engine is stopped.
        /// Returns the exit code (0 = success).
        i32 Run();

        /// Signals the engine to stop.  The main loop will exit after
        /// the current frame completes.
        void RequestStop();

        // ----------------------------------------------------------------
        // State
        // ----------------------------------------------------------------

        /// Returns the current engine state.
        [[nodiscard]] EngineState GetState() const noexcept;

        /// Returns true if the engine is in the Running state.
        [[nodiscard]] bool IsRunning() const noexcept;

        // ----------------------------------------------------------------
        // Accessors
        // ----------------------------------------------------------------

        /// Access the engine context.  Subsystems receive a reference
        /// during initialization and should store it for later queries.
        [[nodiscard]] EngineContext& GetContext() noexcept
        {
            return m_context;
        }

        /// Access the engine context (const).
        [[nodiscard]] const EngineContext& GetContext() const noexcept
        {
            return m_context;
        }

        /// Access the subsystem manager.
        [[nodiscard]] SubsystemManager& GetSubsystemManager() noexcept
        {
            return m_context.GetSubsystemManager();
        }

        /// Access the subsystem manager (const).
        [[nodiscard]] const SubsystemManager& GetSubsystemManager() const noexcept
        {
            return m_context.GetSubsystemManager();
        }

        /// Access the module manager.
        [[nodiscard]] ModuleManager& GetModuleManager() noexcept
        {
            return m_moduleManager;
        }

        /// Access the module manager (const).
        [[nodiscard]] const ModuleManager& GetModuleManager() const noexcept
        {
            return m_moduleManager;
        }

        /// Access frame statistics.
        [[nodiscard]] FrameStats& GetFrameStats() noexcept
        {
            return m_context.GetFrameStats();
        }

        /// Access frame statistics (const).
        [[nodiscard]] const FrameStats& GetFrameStats() const noexcept
        {
            return m_context.GetFrameStats();
        }

        /// Access the engine configuration.
        [[nodiscard]] const EngineConfig& GetConfig() const noexcept
        {
            return m_context.GetConfig();
        }

        // ----------------------------------------------------------------
        // Reporting
        // ----------------------------------------------------------------

        /// Logs a summary of the engine version, build info, and
        /// platform diagnostics.  Called automatically during
        /// initialization when config.printBuildInfo is true.
        void ReportDiagnostics() const;

    private:
        // ----------------------------------------------------------------
        // Internal loop helpers
        // ----------------------------------------------------------------

        /// Processes one frame: tick timing, fixed update loop,
        /// variable update, late update, end frame.
        void ProcessFrame();

        /// Applies engine configuration to subsystems and frame stats.
        void ApplyConfig(const EngineConfig& config);

        /// Attempts to load an external engine configuration file.
        void LoadConfigFile(const EngineConfig& config);

        /// Performs a validated state transition.
        /// Returns true if the transition was legal and applied.
        bool TransitionTo(EngineState newState);

        // ----------------------------------------------------------------
        // Data
        // ----------------------------------------------------------------

        EngineContext  m_context;
        ModuleManager  m_moduleManager;
        EngineConfig   m_config;
        bool           m_initialized = false;
        bool           m_running     = false;
    };

} // namespace engine::runtime