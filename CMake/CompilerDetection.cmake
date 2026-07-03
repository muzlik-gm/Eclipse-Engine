# Compiler detection and configuration for the Engine project.

set(ENGINE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD ${ENGINE_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    set(ENGINE_COMPILER_MSVC 1)
    set(ENGINE_COMPILER_NAME "MSVC")
    add_compile_definitions(ENGINE_COMPILER_MSVC)

    set(ENGINE_COMPILER_FLAGS
        /W4
        /permissive-
        /Zc:__cplusplus
        /Zc:preprocessor
        /utf-8
        /bigobj
    )

    if(ENGINE_WARNINGS_AS_ERRORS)
        list(APPEND ENGINE_COMPILER_FLAGS /WX)
    endif()

    add_compile_options(${ENGINE_COMPILER_FLAGS})

    # Suppress specific MSVC warnings
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(ENGINE_COMPILER_GCC 1)
    set(ENGINE_COMPILER_NAME "GCC")
    add_compile_definitions(ENGINE_COMPILER_GCC)

    set(ENGINE_COMPILER_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    if(ENGINE_WARNINGS_AS_ERRORS)
        list(APPEND ENGINE_COMPILER_FLAGS -Werror)
    endif()

    add_compile_options(${ENGINE_COMPILER_FLAGS})

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(ENGINE_COMPILER_CLANG 1)
    set(ENGINE_COMPILER_NAME "Clang")
    add_compile_definitions(ENGINE_COMPILER_CLANG)

    set(ENGINE_COMPILER_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    if(ENGINE_WARNINGS_AS_ERRORS)
        list(APPEND ENGINE_COMPILER_FLAGS -Werror)
    endif()

    add_compile_options(${ENGINE_COMPILER_FLAGS})

else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}. Using default flags.")
endif()

message(STATUS "Detected compiler: ${ENGINE_COMPILER_NAME} (${CMAKE_CXX_COMPILER_VERSION})")
message(STATUS "C++ standard: C++${ENGINE_CXX_STANDARD}")