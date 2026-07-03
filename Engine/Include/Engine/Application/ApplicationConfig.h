// ============================================================================
// File: Engine/Include/Engine/Application/ApplicationConfig.h
// Application-level configuration that extends the engine configuration.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Runtime/EngineConfig.h"

#include <string>
#include <string_view>

namespace engine::application
{

    // ========================================================================
    // ApplicationConfig
    // ========================================================================

    /// Application-level configuration.  Merged with EngineConfig to
    /// produce the final configuration passed to Engine::Initialize().
    struct ApplicationConfig
    {
        // ----------------------------------------------------------------
        // Engine config (forwarded)
        // ----------------------------------------------------------------

        /// Base engine configuration.  The Application may override
        /// values based on command-line arguments or app-level defaults.
        runtime::EngineConfig engineConfig;

        // ----------------------------------------------------------------
        // Application-specific settings
        // ----------------------------------------------------------------

        /// Path to an application configuration file (JSON or YAML).
        /// Loaded after the engine config file.
        std::string appConfigFilePath;

        /// Whether to run in headless mode (no window, no rendering).
        /// Always true in Phase 1 since no rendering exists.
        bool headless = true;
    };

} // namespace engine::application