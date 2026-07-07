// ============================================================================
// File: Engine/Source/Core/Log.cpp
// Implementation of the engine logging system.
// ============================================================================

#include "Engine/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace engine::core {

std::shared_ptr<spdlog::logger> Log::s_coreLogger = nullptr;
std::shared_ptr<spdlog::logger> Log::s_clientLogger = nullptr;
bool Log::s_initialized = false;

void Log::Initialize(std::string_view appName) {
    if (s_initialized) {
        return;
    }

    // Create sinks: console output and a log file.
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("%^[%L]%$ %v");

    std::string logFileName = std::string(appName) + ".log";
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, true);
    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [thread %t] %v");

    // Create the core logger with both sinks.
    std::vector<spdlog::sink_ptr> coreSinks{consoleSink, fileSink};
    s_coreLogger = std::make_shared<spdlog::logger>("ENGINE", coreSinks.begin(), coreSinks.end());
    s_coreLogger->set_level(spdlog::level::trace);
    s_coreLogger->flush_on(spdlog::level::warn);  // Flush on warnings and above
    spdlog::register_logger(s_coreLogger);

    // Create a separate client logger (for user/game code).
    std::vector<spdlog::sink_ptr> clientSinks{consoleSink, fileSink};
    s_clientLogger = std::make_shared<spdlog::logger>("APP", clientSinks.begin(), clientSinks.end());
    s_clientLogger->set_level(spdlog::level::trace);
    s_clientLogger->flush_on(spdlog::level::critical);
    spdlog::register_logger(s_clientLogger);

    // Set the default logger to the core logger.
    spdlog::set_default_logger(s_coreLogger);

    // Adjust default level based on build configuration.
#ifdef ENGINE_DEBUG
    s_coreLogger->set_level(spdlog::level::trace);
    s_clientLogger->set_level(spdlog::level::trace);
#else
    s_coreLogger->set_level(spdlog::level::info);
    s_clientLogger->set_level(spdlog::level::info);
#endif

    s_initialized = true;
    s_coreLogger->info("Logging system initialized. App: {}", appName);
}

void Log::Shutdown() {
    if (!s_initialized) {
        return;
    }

    s_coreLogger->info("Logging system shutting down.");
    s_coreLogger->flush();
    s_clientLogger->flush();

    spdlog::drop("ENGINE");
    spdlog::drop("APP");

    s_coreLogger.reset();
    s_clientLogger.reset();
    s_initialized = false;
}

void Log::SetLevel(LogLevel level) {
    if (!s_coreLogger) {
        return;
    }

    auto spdlogLevel = static_cast<spdlog::level::level_enum>(level);
    s_coreLogger->set_level(spdlogLevel);
    if (s_clientLogger) {
        s_clientLogger->set_level(spdlogLevel);
    }
}

LogLevel Log::GetLevel() {
    if (!s_coreLogger) {
        return LogLevel::Off;
    }
    return static_cast<LogLevel>(s_coreLogger->level());
}

void Log::SetPattern(std::string_view pattern) {
    if (!s_coreLogger) {
        return;
    }
    auto patternStr = std::string(pattern);
    s_coreLogger->set_pattern(patternStr);
    if (s_clientLogger) {
        s_clientLogger->set_pattern(patternStr);
    }
}

void Log::Flush() {
    if (s_coreLogger) {
        s_coreLogger->flush();
    }
    if (s_clientLogger) {
        s_clientLogger->flush();
    }
}

std::shared_ptr<spdlog::logger> Log::GetCoreLogger() {
    return s_coreLogger;
}

std::shared_ptr<spdlog::logger> Log::GetClientLogger() {
    return s_clientLogger;
}

} // namespace engine::core