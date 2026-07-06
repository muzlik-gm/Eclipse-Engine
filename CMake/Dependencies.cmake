# Dependency management using FetchContent for the Engine project.
#
# Dependencies are declared here and fetched automatically.
# Phase 0 requires: spdlog, nlohmann_json, yaml-cpp, glm, entt, googletest.
# Future-phase dependencies (GLFW, GLAD, ImGui, stb, tinyobjloader) are
# declared for automatic download but only made available (built) when the
# corresponding engine phase is implemented.

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

# ============================================================================
# Phase 0 Dependencies (actively built and linked)
# ============================================================================

# --- spdlog (logging) ---
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

# --- fmt (formatting, spdlog dependency but we use it too) ---
# spdlog already pulls fmt; we reference it via alias

# --- nlohmann/json (JSON configuration) ---
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# --- yaml-cpp (YAML configuration) ---
# Force-disable extras as cache variables before fetching (most reliable method)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DYAML_BUILD_SHARED_LIBS=OFF
        -DYAML_CPP_BUILD_TESTS=OFF
        -DYAML_CPP_BUILD_TOOLS=OFF
        -DYAML_CPP_BUILD_CONTRIB=OFF
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)
FetchContent_MakeAvailable(yaml-cpp)

# --- GLM (math library, header-only) ---
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# --- EnTT (ECS, header-only) ---
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.13.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(entt)

# --- Google Test (for testing) ---
if(ENGINE_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
        GIT_SHALLOW TRUE
        CMAKE_ARGS
            -Dgtest_force_shared_crt=ON
            -Dgtest_disable_pthreads=OFF
    )
    FetchContent_MakeAvailable(googletest)
endif()

# ============================================================================
# Future-Phase Dependencies (declared for automatic download, not yet built)
# These will be made available (FetchContent_MakeAvailable) when their
# corresponding engine phase is implemented.
# ============================================================================

# --- GLFW (window) --- Phase 2: Platform Layer & Window System ---
# Set up X11 compatibility headers BEFORE FetchContent so GLFW's
# configure step can find the RandR/Xinerama/Xcursor extension headers.
set(GLFW_X11_COMPAT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/_glfw_x11_compat")

# Cross-platform patch: force-disable tests/examples/docs in GLFW's
# CMakeLists.txt AND (on Linux) inject X11 compat include paths.
# Using cmake -P script ensures it works on Windows/macOS/Linux alike.
if(ENGINE_PLATFORM_LINUX AND NOT APPLE)
    file(MAKE_DIRECTORY "${GLFW_X11_COMPAT_DIR}/X11/Xcursor")
    file(MAKE_DIRECTORY "${GLFW_X11_COMPAT_DIR}/X11/extensions")
    set(_GLFW_PATCH_CMD
        ${CMAKE_COMMAND}
        -DSRC_FILE=<SOURCE_DIR>/CMakeLists.txt
        -DCOMPAT_DIR=${GLFW_X11_COMPAT_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/patch_glfw.cmake)
else()
    set(_GLFW_PATCH_CMD
        ${CMAKE_COMMAND}
        -DSRC_FILE=<SOURCE_DIR>/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/patch_glfw.cmake)
endif()

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
    GIT_SHALLOW TRUE
    PATCH_COMMAND ${_GLFW_PATCH_CMD}
    CMAKE_ARGS
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_DOCS=OFF
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)
FetchContent_MakeAvailable(glfw)

# Add compatibility include path for X11 extension headers for compilation.
if(ENGINE_PLATFORM_LINUX AND NOT APPLE)
    target_include_directories(glfw BEFORE PRIVATE "${GLFW_X11_COMPAT_DIR}")
endif()

# --- GLAD (OpenGL loader) --- Phase 4: OpenGL Backend ---
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glad)

# Generate GLAD for OpenGL 4.6 Core Profile using the Python generator.
# v0.1.36 does not provide glad_add_library, so we generate sources manually.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(GLAD_GENERATE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/glad-generated")

add_custom_command(
    OUTPUT
        "${GLAD_GENERATE_DIR}/src/glad.c"
        "${GLAD_GENERATE_DIR}/include/glad/gl.h"
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${glad_SOURCE_DIR}"
        ${Python3_EXECUTABLE} -m glad
        --generator c
        --out-path "${GLAD_GENERATE_DIR}"
        --api "gl=4.6"
        --spec gl
        --extensions "GL_KHR_debug GL_ARB_debug_output GL_ARB_buffer_storage GL_ARB_direct_state_access GL_ARB_seamless_cubemap_per_texture GL_ARB_texture_storage"
        --profile core
    DEPENDS ${glad_SOURCE_DIR}/glad/__main__.py
    COMMENT "Generating GLAD OpenGL 4.6 Core Profile loader"
    VERBATIM
)

add_library(glad_api STATIC "${GLAD_GENERATE_DIR}/src/glad.c")
target_include_directories(glad_api
    PUBLIC
        "${GLAD_GENERATE_DIR}/include"
)
target_compile_features(glad_api PUBLIC cxx_std_20)

if(ENGINE_PLATFORM_LINUX AND NOT APPLE)
    target_link_libraries(glad_api PRIVATE dl)
endif()

# --- Dear ImGui (editor UI) — Phase 7: Editor Foundation ---
# Use the docking branch for dockspace + multi-viewport support.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

# Build ImGui as a static library with GLFW + OpenGL3 backends.
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    # Backends
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui
    PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui
    PUBLIC
        glfw
        glad_api
)
target_compile_features(imgui PUBLIC cxx_std_20)
target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
target_compile_definitions(imgui PUBLIC GLFW_INCLUDE_NONE)

# --- stb (image loading) — Phase 14: Importer Pipeline ---
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG f2008cb
    GIT_SHALLOW TRUE
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

# --- tinyobjloader (mesh import) — Phase 14: Importer Pipeline ---
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG v2.0.0rc13
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DTINYOBJLOADER_BUILD_TEST_LOADER=OFF
        -DTINYOBJLOADER_COMPILE_EXAMPLES=OFF
)