# patch_glfw.cmake
# Cross-platform GLFW CMakeLists.txt patcher.
# - On Linux (non-macOS): also injects X11 compat include paths.
#
# Usage:
#   cmake -DSRC_FILE=<glfw_CMakeLists.txt> [-DCOMPAT_DIR=<x11_compat_dir>] -P patch_glfw.cmake

file(READ "${SRC_FILE}" _content)

# ---------------------------------------------------------------
# 1. Force-disable tests, examples, and docs by replacing the
#    option() defaults with OFF and adding set(... FORCE).
# ---------------------------------------------------------------

string(REPLACE
    "option(GLFW_BUILD_TESTS \"Build the GLFW test programs\" ON)"
    "option(GLFW_BUILD_TESTS \"Build the GLFW test programs\" OFF)"
    _content "${_content}")

string(REPLACE
    "option(GLFW_BUILD_EXAMPLES \"Build the GLFW example programs\" ON)"
    "option(GLFW_BUILD_EXAMPLES \"Build the GLFW example programs\" OFF)"
    _content "${_content}")

string(REPLACE
    "option(GLFW_BUILD_DOCS \"Build the GLFW documentation\" ON)"
    "option(GLFW_BUILD_DOCS \"Build the GLFW documentation\" OFF)"
    _content "${_content}")

# Add FORCE overrides right after the project() call so cache
# values from a previous configure can't re-enable them.
string(REPLACE
    "option(GLFW_BUILD_TESTS "
    "set(GLFW_BUILD_TESTS OFF CACHE BOOL \"\" FORCE)\noption(GLFW_BUILD_TESTS "
    _content "${_content}")

string(REPLACE
    "option(GLFW_BUILD_EXAMPLES "
    "set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL \"\" FORCE)\noption(GLFW_BUILD_EXAMPLES "
    _content "${_content}")

string(REPLACE
    "option(GLFW_BUILD_DOCS "
    "set(GLFW_BUILD_DOCS OFF CACHE BOOL \"\" FORCE)\noption(GLFW_BUILD_DOCS "
    _content "${_content}")

# ---------------------------------------------------------------
# 2. On Linux: inject X11 compat include path before find_package(X11).
# ---------------------------------------------------------------

if(DEFINED COMPAT_DIR AND COMPAT_DIR)
    string(REPLACE
        "# Use X11 for window creation"
        "set(CMAKE_PREFIX_PATH \"${COMPAT_DIR}\" CACHE PATH \"\" FORCE)\ninclude_directories(BEFORE \"${COMPAT_DIR}\")\n\n# Use X11 for window creation"
        _content "${_content}")
endif()

file(WRITE "${SRC_FILE}" "${_content}")