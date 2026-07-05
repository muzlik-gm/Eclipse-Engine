cmake_minimum_required(VERSION 3.22)

# Allow sub-projects that declare older cmake_minimum_required to build
# with this newer CMake version (4.x removed backward-compat support).
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(Engine
    VERSION 0.1.0
    DESCRIPTION "A professional, modular, cross-platform C++20 game engine"
    LANGUAGES C CXX
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

if(ENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

if(ENGINE_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif()

if(ENGINE_BUILD_EDITOR)
    add_subdirectory(Editor)
endif()