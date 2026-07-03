# Sandbox application — minimal executable that exercises the Phase 1
# runtime architecture (Application + Engine + main loop).

set(SANDBOX_SOURCES
    Source/SandboxMain.cpp
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox
    PRIVATE
        Engine
)

set_target_properties(Sandbox PROPERTIES
    FOLDER "Sandbox"
    OUTPUT_NAME "Sandbox"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)