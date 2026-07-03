cmake_minimum_required(VERSION 3.22)

project(Engine
    VERSION 0.1.0
    DESCRIPTION "A professional, modular, cross-platform C++20 game engine"
    LANGUAGES CXX
)

# CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(BuildOptions)
include(PlatformDetection)
include(CompilerDetection)

# Dependencies
include(Dependencies)

# Sub-projects
add_subdirectory(Engine)

option(ENGINE_BUILD_TESTS "Build engine tests" ON)
if(ENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

option(ENGINE_BUILD_SANDBOX "Build sandbox application" OFF)
if(ENGINE_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif()

option(ENGINE_BUILD_EDITOR "Build editor" OFF)
if(ENGINE_BUILD_EDITOR)
    add_subdirectory(Editor)
endif()