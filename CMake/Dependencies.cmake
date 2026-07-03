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
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DYAML_BUILD_SHARED_LIBS=OFF
        -DYAML_CPP_BUILD_TESTS=OFF
        -DYAML_CPP_BUILD_TOOLS=OFF
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

# --- GLFW (window) — Phase 1: Platform Foundation ---
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_DOCS=OFF
)

# --- GLAD (OpenGL loader) — Phase 6: OpenGL Backend ---
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
    GIT_SHALLOW TRUE
)

# --- Dear ImGui (editor UI) — Phase 27: Dear ImGui Integration ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DIMGUI_BUILD_TESTS=OFF
)

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