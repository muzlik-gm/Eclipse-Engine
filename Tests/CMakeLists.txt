# Test infrastructure for the Engine project.

set(ENGINE_TEST_SOURCES
    Source/Core/LogTests.cpp
    Source/Core/UUIDTests.cpp
    Source/Core/TimingTests.cpp
    Source/Filesystem/PathTests.cpp
    Source/Configuration/ConfigTests.cpp
    Source/Utilities/StringTests.cpp
    Source/Utilities/HashTests.cpp
    Source/Utilities/CommandLineTests.cpp
    Source/Threading/ThreadTests.cpp
)

add_executable(EngineTests ${ENGINE_TEST_SOURCES})

target_link_libraries(EngineTests
    PRIVATE
        Engine
        GTest::gtest_main
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