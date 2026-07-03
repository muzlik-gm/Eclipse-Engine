#pragma once

/**
 * @file Engine.h
 * @brief Master include for the entire engine public API.
 *
 * Including this single header pulls in every public module that the engine
 * exposes.  Use it in application code, samples, and the editor — anywhere
 * you want the full engine surface area available without managing individual
 * includes.
 *
 * Internal engine source files that only need a subset should prefer
 * including the specific module headers (or Core/Base.h) to keep
 * compilation times minimal.
 */

// Core fundamentals (types, platform, compiler, build config, version)
#include "Engine/Core/Base.h"

// Core subsystems
#include "Engine/Core/Log.h"
#include "Engine/Core/Assert.h"
#include "Engine/Core/UUID.h"
#include "Engine/Core/Timing.h"

// Diagnostics — profiling scopes, performance counters, build metadata
#include "Engine/Diagnostics/Instrumentation.h"

// Filesystem — cross-platform path manipulation and file I/O
#include "Engine/Filesystem/Path.h"

// Configuration — runtime config loading and typed value access
#include "Engine/Configuration/Config.h"

// Threading — thread abstraction, CPU topology queries
#include "Engine/Threading/Thread.h"