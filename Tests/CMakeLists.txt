# Test infrastructure for the Engine project.

set(ENGINE_TEST_SOURCES
    Source/Core/LogTests.cpp
    Source/Core/UUIDTests.cpp
    Source/Core/TimingTests.cpp
    Source/Core/ModuleRegistryTests.cpp
    Source/Filesystem/PathTests.cpp
    Source/Filesystem/FileTests.cpp
    Source/Filesystem/DirectoryTests.cpp
    Source/Configuration/ConfigTests.cpp
    Source/Utilities/StringTests.cpp
    Source/Utilities/HashTests.cpp
    Source/Utilities/CommandLineTests.cpp
    Source/Threading/ThreadTests.cpp
    Source/Diagnostics/BuildInfoTests.cpp
    Source/Diagnostics/SystemInfoTests.cpp
    Source/Diagnostics/PerformanceCountersTests.cpp
)

add_executable(EngineTests ${ENGINE_TEST_SOURCES})

target_include_directories(EngineTests PRIVATE
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
)

target_link_libraries(EngineTests
    PRIVATE
        Engine
        gtest
        gtest_main
)

target_compile_definitions(EngineTests PRIVATE
    ENGINE_TESTING
)

set_target_properties(EngineTests PROPERTIES
    FOLDER "Tests"
    OUTPUT_NAME "EngineTests"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

include(GoogleTest)
gtest_discover_tests(EngineTests)