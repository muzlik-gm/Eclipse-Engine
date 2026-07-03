// ============================================================================
// File: Engine/Include/Engine/Runtime/EngineConfig.h
// Configuration parameters that control engine runtime behaviour.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"

#include <string>
#include <string_view>

namespace engine::runtime
{

    using engine::core::f64;
    using engine::core::u32;
    using engine::core::usize;

    // ========================================================================
    // EngineConfig
    // ========================================================================

    /// Holds all tuneable parameters that influence engine runtime behaviour.
    ///
    /// These values can be loaded from a configuration file at startup or
    /// set programmatically before Engine::Initialize() is called.
    /// Sensible defaults are provided for every member.
    struct EngineConfig
    {
        // --------------------------------------------------------------------
        // Timing
        // --------------------------------------------------------------------

        /// Fixed timestep in seconds.  Default: 1/60 (~16.67 ms).
        f64 fixedDeltaTime = 1.0 / 60.0;

        /// Maximum fixed-update steps per frame to prevent the spiral of
        /// death.  Default: 5.
        usize maxFixedStepsPerFrame = 5;

        /// Target frame time in seconds for frame-rate limiting.
        /// 0.0 means no limiting (run as fast as possible).
        f64 targetFrameTime = 0.0;

        /// Initial time scale factor.  1.0 = real-time.
        f64 timeScale = 1.0;

        // --------------------------------------------------------------------
        // Logging
        // --------------------------------------------------------------------

        /// Minimum log level at engine startup.
        /// 0 = Trace, 1 = Debug, 2 = Info, 3 = Warning, 4 = Error, 5 = Critical.
        u32 logLevel = 2;

        /// Whether to write logs to a file in addition to the console.
        bool enableFileLogging = true;

        /// Path to the log output file.  Empty uses the default path.
        std::string logFilePath;

        // --------------------------------------------------------------------
        // Configuration
        // --------------------------------------------------------------------

        /// Path to an engine configuration file (JSON or YAML) that is
        /// loaded after command-line arguments are parsed.  Empty means
        /// no file is loaded.
        std::string configFilePath;

        // --------------------------------------------------------------------
        // Diagnostics
        // --------------------------------------------------------------------

        /// Whether to print full build/diagnostic information at startup.
        bool printBuildInfo = true;

        /// Whether to print a summary of all registered subsystems at startup.
        bool printSubsystemInfo = false;
    };

} // namespace engine::runtime