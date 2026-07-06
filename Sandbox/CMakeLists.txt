# Sandbox application — renders a colored triangle using the engine
# platform layer + OpenGL backend.

set(SANDBOX_SOURCES
    Source/SandboxMain.cpp
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox
    PRIVATE
        Engine
        glad_api
        glfw
)

set_target_properties(Sandbox PROPERTIES
    FOLDER "Sandbox"
    OUTPUT_NAME "Sandbox"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
