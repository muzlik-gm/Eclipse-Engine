/// @file Assert.cpp
/// @brief Implementation of the engine assertion-failure handler.

#include "Engine/Core/Assert.h"

#include <spdlog/spdlog.h>

#include <cstdlib>

namespace engine::core::detail {

void AssertFail(const char* condition,
                const char* message,
                const char* file,
                int         line,
                const char* function)
{
    spdlog::critical(
        "ASSERTION FAILED: {}\n"
        "  Message: {}\n"
        "  File: {}:{}\n"
        "  Function: {}",
        condition ? condition : "<null>",
        message   ? message   : "",
        file      ? file      : "<unknown>",
        line,
        function  ? function  : "<unknown>");

    spdlog::default_logger()->flush();

    // Platform-specific debug trap / termination.
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#else
    std::abort();
#endif
}

} // namespace engine::core::detail