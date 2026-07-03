#pragma once

/**
 * @file Base.h
 * @brief Convenience umbrella for the Core module's foundational headers.
 *
 * Including this single header gives you platform detection, compiler
 * feature flags, build configuration, all type aliases, and the engine
 * version — everything a typical engine source file needs to get started.
 */

#include "Engine/Core/Types.h"
#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/Compiler.h"
#include "Engine/Core/Version.h"