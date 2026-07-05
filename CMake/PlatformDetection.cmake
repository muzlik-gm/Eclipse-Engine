# Platform detection for the Engine project.

if(WIN32)
    set(ENGINE_PLATFORM_WINDOWS 1)
    set(ENGINE_PLATFORM_NAME "Windows")
    add_compile_definitions(ENGINE_PLATFORM_WINDOWS=1)
elseif(APPLE)
    set(ENGINE_PLATFORM_MACOS 1)
    set(ENGINE_PLATFORM_NAME "macOS")
    add_compile_definitions(ENGINE_PLATFORM_MACOS=1)
elseif(UNIX)
    set(ENGINE_PLATFORM_LINUX 1)
    set(ENGINE_PLATFORM_NAME "Linux")
    add_compile_definitions(ENGINE_PLATFORM_LINUX=1)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

message(STATUS "Detected platform: ${ENGINE_PLATFORM_NAME}")