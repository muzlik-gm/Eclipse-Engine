# Build configuration options for the Engine project.

option(ENGINE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ENGINE_ENABLE_ASSERTIONS "Enable engine assertions" ON)
option(ENGINE_ENABLE_PROFILING "Enable profiling instrumentation" ON)
option(ENGINE_ENABLE_LOGGING "Enable logging system" ON)
option(ENGINE_BUILD_SHARED "Build engine as shared library" OFF)
option(ENGINE_BUILD_TESTS "Build engine tests" ON)
option(ENGINE_BUILD_SANDBOX "Build sandbox application" ON)
option(ENGINE_BUILD_EDITOR "Build editor" ON)

if(ENGINE_BUILD_SHARED)
    set(ENGINE_LIBRARY_TYPE SHARED)
else()
    set(ENGINE_LIBRARY_TYPE STATIC)
endif()