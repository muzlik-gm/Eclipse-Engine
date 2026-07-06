# Engine library target definition.

set(ENGINE_SOURCES
    # Core
    Source/Core/Log.cpp
    Source/Core/Assert.cpp
    Source/Core/UUID.cpp
    Source/Core/Timing.cpp
    # Diagnostics
    Source/Diagnostics/Instrumentation.cpp
    Source/Diagnostics/BuildInfo.cpp
    Source/Diagnostics/SystemInfo.cpp
    Source/Diagnostics/PerformanceCounters.cpp
    Source/Diagnostics/StackTrace.cpp
    Source/Diagnostics/CrashHandler.cpp
    Source/Diagnostics/MemoryStats.cpp
    # Filesystem
    Source/Filesystem/Path.cpp
    Source/Filesystem/File.cpp
    Source/Filesystem/Directory.cpp
    # Configuration
    Source/Configuration/Config.cpp
    # Threading
    Source/Threading/Thread.cpp
    Source/Threading/CPUInfo.cpp
    # Utilities
    Source/Utilities/Hash.cpp
    Source/Utilities/String.cpp
    Source/Utilities/CommandLine.cpp
    # Module
    Source/Core/ModuleRegistry.cpp
    # Runtime (Phase 1)
    Source/Runtime/FrameStats.cpp
    Source/Runtime/SubsystemManager.cpp
    Source/Runtime/ModuleManager.cpp
    Source/Runtime/Engine.cpp
    # Application (Phase 1)
    Source/Application/Application.cpp
    # Platform (Phase 2)
    Source/Platform/PlatformManager.cpp
    Source/Platform/GLFW/GLFWWindow.cpp
    Source/Platform/GLFW/GLFWMonitor.cpp
    Source/Platform/GLFW/GLFWClipboard.cpp
    Source/Platform/GLFW/GLFWCursor.cpp
    Source/Platform/GLFW/GLFWDynamicLibrary.cpp
    Source/Platform/PlatformInfo.cpp
    Source/Platform/FileDialogStub.cpp
    Source/Platform/Timer.cpp
    # Events (Phase 2)
    Source/Events/EventBus.cpp
    # World, Scene & ECS (Phase 5)
    Source/Entities/EntityManager.cpp
    Source/Systems/TransformSystem.cpp
    Source/Systems/SystemScheduler.cpp
    Source/Scene/Scene.cpp
    Source/Scene/SceneManager.cpp
    Source/World/World.cpp
    Source/World/WorldManager.cpp
    Source/Serialization/SceneSerializer.cpp
    # OpenGL Backend (Phase 4)
    Source/Renderer/OpenGL/OpenGLTypes.cpp
    Source/Renderer/OpenGL/OpenGLDebugLayer.cpp
    Source/Renderer/OpenGL/OpenGLStateCache.cpp
    Source/Renderer/OpenGL/OpenGLContext.cpp
    Source/Renderer/OpenGL/OpenGLCapabilities.cpp
    Source/Renderer/OpenGL/OpenGLBuffer.cpp
    Source/Renderer/OpenGL/OpenGLTexture.cpp
    Source/Renderer/OpenGL/OpenGLShaderPreprocessor.cpp
    Source/Renderer/OpenGL/OpenGLShader.cpp
    Source/Renderer/OpenGL/OpenGLPipeline.cpp
    Source/Renderer/OpenGL/OpenGLVertexArray.cpp
    Source/Renderer/OpenGL/OpenGLRenderPass.cpp
    Source/Renderer/OpenGL/OpenGLSync.cpp
    Source/Renderer/OpenGL/OpenGLCommandBuffer.cpp
    Source/Renderer/OpenGL/OpenGLCommandQueue.cpp
    Source/Renderer/OpenGL/OpenGLSwapChain.cpp
    Source/Renderer/OpenGL/OpenGLFactory.cpp
    Source/Renderer/OpenGL/OpenGLDevice.cpp
    Source/Renderer/OpenGL/OpenGLBackend.cpp
    # Render Core (Phase 4)
    Source/Renderer/Core/RendererStatistics.cpp
    Source/Renderer/Core/RendererContext.cpp
    Source/Renderer/Core/Renderer.cpp
)

add_library(Engine ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCES})

target_include_directories(Engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_BINARY_DIR}/_deps/glad-generated/include
)

# Link dependencies
find_package(Threads REQUIRED)

target_link_libraries(Engine
    PRIVATE
        yaml-cpp::yaml-cpp
        Threads::Threads
        glfw
        glad_api
    PUBLIC
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        glm::glm
        EnTT::EnTT
)

# Compile definitions based on build type
target_compile_definitions(Engine PUBLIC
    $<$<CONFIG:Debug>:ENGINE_DEBUG>
    $<$<CONFIG:Release>:ENGINE_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:ENGINE_RELEASE>
    $<$<CONFIG:MinSizeRel>:ENGINE_RELEASE>
)

if(ENGINE_ENABLE_ASSERTIONS)
    target_compile_definitions(Engine PUBLIC ENGINE_ENABLE_ASSERTIONS)
endif()

if(ENGINE_ENABLE_PROFILING)
    target_compile_definitions(Engine PUBLIC ENGINE_ENABLE_PROFILING)
endif()

if(ENGINE_ENABLE_LOGGING)
    target_compile_definitions(Engine PUBLIC ENGINE_ENABLE_LOGGING)
endif()

# Platform-specific link libraries
if(ENGINE_PLATFORM_LINUX)
    target_link_libraries(Engine PRIVATE dl rt)
    # Enable symbol resolution for StackTrace backtrace_symbols
    set_property(TARGET Engine APPEND_STRING PROPERTY LINK_FLAGS " -rdynamic")
elseif(ENGINE_PLATFORM_MACOS)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(Engine PRIVATE ${COREFOUNDATION_LIBRARY})
endif()

# IDE folder organization
set_target_properties(Engine PROPERTIES
    FOLDER "Engine"
    OUTPUT_NAME "Engine"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Include" PREFIX "Headers" FILES ${ENGINE_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" PREFIX "Source" FILES ${ENGINE_SOURCES})