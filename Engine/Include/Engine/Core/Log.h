#pragma once
// ============================================================================
// File: Engine/Include/Engine/Core/Log.h
// Engine logging system built on spdlog.
// Provides typed log levels, multi-sink support (console + file),
// and a global engine-wide logging API.
// ============================================================================

#include "Engine/Core/BuildConfig.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>
#include <string>
#include <string_view>

namespace engine::core {

/// Log severity levels, matching spdlog levels.
enum class LogLevel : std::uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5,
    Off = 6
};

/// Global logging manager. Wraps spdlog with engine-specific initialization
/// and provides a clean API for all engine subsystems.
class Log {
public:
    /// Initialize the logging system with console and file sinks.
    /// Call this once at application startup before any logging occurs.
    static void Initialize(std::string_view appName = "Engine");

    /// Shut down the logging system and flush all pending messages.
    static void Shutdown();

    /// Set the minimum log level for the core logger.
    static void SetLevel(LogLevel level);

    /// Get the current minimum log level.
    [[nodiscard]] static LogLevel GetLevel();

    /// Change the log output pattern. See spdlog pattern documentation.
    static void SetPattern(std::string_view pattern);

    /// Flush all pending log messages to their sinks.
    static void Flush();

    /// Access the underlying spdlog core logger for advanced use.
    [[nodiscard]] static std::shared_ptr<spdlog::logger> GetCoreLogger();

    /// Access the underlying spdlog client logger for advanced use.
    [[nodiscard]] static std::shared_ptr<spdlog::logger> GetClientLogger();

    template <typename... Args>
    static void Trace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_coreLogger) {
            s_coreLogger->trace(fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void Debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_coreLogger) {
            s_coreLogger->debug(fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void Info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_coreLogger) {
            s_coreLogger->info(fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void Warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_coreLogger) {
            s_coreLogger->warn(fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void Error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_coreLogger) {
            s_coreLogger->error(fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void Critical(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_coreLogger) {
            s_coreLogger->critical(fmt, std::forward<Args>(args)...);
        }
    }

private:
    static std::shared_ptr<spdlog::logger> s_coreLogger;
    static std::shared_ptr<spdlog::logger> s_clientLogger;
    static bool s_initialized;
};

} // namespace engine::core

// Convenience macros for logging throughout the engine codebase.
#define ENGINE_LOG_TRACE(...)    ::engine::core::Log::Trace(__VA_ARGS__)
#define ENGINE_LOG_DEBUG(...)    ::engine::core::Log::Debug(__VA_ARGS__)
#define ENGINE_LOG_INFO(...)     ::engine::core::Log::Info(__VA_ARGS__)
#define ENGINE_LOG_WARN(...)     ::engine::core::Log::Warn(__VA_ARGS__)
#define ENGINE_LOG_ERROR(...)    ::engine::core::Log::Error(__VA_ARGS__)
#define ENGINE_LOG_CRITICAL(...) ::engine::core::Log::Critical(__VA_ARGS__)