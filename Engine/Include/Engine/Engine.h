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

// Platform — windowing, monitor, clipboard, cursor, dynamic library, file dialog
#include "Engine/Platform/Window.h"
#include "Engine/Platform/OS.h"
#include "Engine/Platform/Monitor.h"
#include "Engine/Platform/Clipboard.h"
#include "Engine/Platform/Cursor.h"
#include "Engine/Platform/DynamicLibrary.h"
#include "Engine/Platform/PlatformInfo.h"
#include "Engine/Platform/FileDialog.h"
#include "Engine/Platform/PlatformManager.h"

// Events — type-safe event system and event bus
#include "Engine/Events/Event.h"
#include "Engine/Events/EventBus.h"

// Memory — custom allocators and memory utilities
#include "Engine/Memory/Allocator.h"
#include "Engine/Memory/Memory.h"

// Math — vector, matrix, quaternion, and common math operations
#include "Engine/Math/Math.h"

// Runtime — engine lifecycle, subsystem/module management, state machine
#include "Engine/Runtime/EngineState.h"
#include "Engine/Runtime/FrameStats.h"
#include "Engine/Runtime/EngineConfig.h"
#include "Engine/Runtime/ISubsystem.h"
#include "Engine/Runtime/SubsystemManager.h"
#include "Engine/Runtime/IModule.h"
#include "Engine/Runtime/ModuleManager.h"
#include "Engine/Runtime/EngineContext.h"
#include "Engine/Runtime/Engine.h"

// Application — application layer, specifications, configuration
#include "Engine/Application/ApplicationSpec.h"
#include "Engine/Application/ApplicationConfig.h"
#include "Engine/Application/Application.h"

// ECS — entity identifier, registry wrapper, and component metadata
#include "Engine/ECS/Entity.h"
#include "Engine/ECS/Registry.h"
#include "Engine/ECS/ComponentRegistry.h"

// Entities — fluent entity handle and manager
#include "Engine/Entities/EntityHandle.h"
#include "Engine/Entities/EntityManager.h"

// Components — core data-only components
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Components/NameComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/HierarchyComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Components/EnabledComponent.h"
#include "Engine/Components/StaticComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/ScriptComponent.h"
#include "Engine/Components/AudioSource.h"
#include "Engine/Components/Collider.h"
#include "Engine/Components/RigidBody.h"

// Systems — base system interface, scheduler, and built-in systems
#include "Engine/Systems/ISystem.h"
#include "Engine/Systems/TransformSystem.h"
#include "Engine/Systems/SystemScheduler.h"

// Transforms — TRS composition, decomposition, local-to-world utilities
#include "Engine/Transforms/TransformUtils.h"

// Hierarchy — parent-child manipulation utilities
#include "Engine/Hierarchy/HierarchyUtils.h"

// Scene — lifecycle, context, self-contained ECS world with systems
#include "Engine/Scene/SceneLifecycle.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/Scene.h"

// World — manager facade and subsystem managing multiple scenes
#include "Engine/World/WorldManager.h"
#include "Engine/World/World.h"

// Events — scene and world lifecycle events
#include "Engine/SceneEvents/SceneEvents.h"
#include "Engine/WorldEvents/WorldEvents.h"

// Serialization — scene serialization interfaces and JSON implementation
#include "Engine/Serialization/Interfaces/ISceneLoader.h"
#include "Engine/Serialization/Interfaces/ISceneSaver.h"
#include "Engine/Serialization/SceneSerializer.h"

// Rendering — abstract render interface stub
#include "Engine/Rendering/IRenderInterface.h"