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
#include "Engine/Core/ModuleRegistry.h"
#include "Engine/Core/Singleton.h"

// Diagnostics — profiling, build info, system info, performance counters, stack traces, crash handling, memory stats
#include "Engine/Diagnostics/Instrumentation.h"
#include "Engine/Diagnostics/BuildInfo.h"
#include "Engine/Diagnostics/SystemInfo.h"
#include "Engine/Diagnostics/PerformanceCounters.h"
#include "Engine/Diagnostics/StackTrace.h"
#include "Engine/Diagnostics/CrashHandler.h"
#include "Engine/Diagnostics/MemoryStats.h"

// Filesystem — cross-platform path manipulation and file I/O
#include "Engine/Filesystem/Path.h"
#include "Engine/Filesystem/File.h"
#include "Engine/Filesystem/Directory.h"

// Configuration — runtime config loading and typed value access
#include "Engine/Configuration/Config.h"

// Threading — thread abstraction, condition variables, CPU topology queries
#include "Engine/Threading/Thread.h"
#include "Engine/Threading/CPUInfo.h"

// Utilities — string, hash, bit manipulation, UTF, type traits, command line, environment
#include "Engine/Utilities/String.h"
#include "Engine/Utilities/Hash.h"
#include "Engine/Utilities/Bit.h"
#include "Engine/Utilities/UTF.h"
#include "Engine/Utilities/TypeTraits.h"
#include "Engine/Utilities/CommandLine.h"
#include "Engine/Utilities/Environment.h"

// Platform — windowing and OS abstraction
#include "Engine/Platform/Window.h"
#include "Engine/Platform/OS.h"

// Events — type-safe event system and event bus
#include "Engine/Events/Event.h"
#include "Engine/Events/EventBus.h"

// Memory — custom allocators and memory utilities
#include "Engine/Memory/Allocator.h"
#include "Engine/Memory/Memory.h"

// Math — vector, matrix, quaternion, and common math operations
#include "Engine/Math/Math.h"