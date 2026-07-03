// ============================================================================
// File: Engine/Include/Engine/Application/ApplicationSpec.h
// Describes the application to be run by the engine.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"

#include <string>
#include <string_view>
#include <vector>

namespace engine::application
{

    // ========================================================================
    // ApplicationSpec
    // ========================================================================

    /// Declarative specification for an Application instance.
    ///
    /// The user (or the EntryPoint macro) fills in this struct before
    /// creating the Application.  The Application uses it to configure
    /// the engine and set up logging.
    struct ApplicationSpec
    {
        /// Human-readable application name.  Used for the logger and
        /// window title (in a future phase).
        std::string name = "Engine Application";

        /// Command-line arguments (argc / argv).  Parsed before engine
        /// initialization.
        std::vector<std::string> commandLineArgs;
    };

} // namespace engine::application