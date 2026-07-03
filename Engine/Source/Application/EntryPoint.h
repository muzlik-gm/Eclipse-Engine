// ============================================================================
// File: Engine/Source/Application/EntryPoint.h
// Main entry abstraction — provides the ENGINE_CREATE_APPLICATION macro
// that users invoke to define their application factory function.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"

/// Defines the application creation function.
///
/// Usage in the user's main.cpp:
///
///   #include "Engine/Application/EntryPoint.h"
///   #include "MyApp.h"
///
///   engine::application::Application* CreateApplication(
///       engine::core::i32 argc, const char* const argv[])
///   {
///       return new MyApp(argc, argv);
///   }
///
/// Then the platform-specific main function (provided by a future phase
/// or by the sandbox) calls CreateApplication().
///
/// The macro simply declares the function with C linkage for potential
/// cross-module usage.
#define ENGINE_DEFINE_APPLICATION(FactoryFunction)                           \
    extern "C" engine::application::Application*                             \
    EngineCreateApplication(engine::core::i32 argc, const char* const argv[]) \
    {                                                                         \
        return FactoryFunction(argc, argv);                                   \
    }