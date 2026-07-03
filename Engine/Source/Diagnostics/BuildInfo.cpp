/**
 * @file BuildInfo.cpp
 * @brief BuildInfo implementation.
 */

#include "Engine/Diagnostics/BuildInfo.h"

#include "Engine/Core/Compiler.h"
#include "Engine/Core/Platform.h"

#include <sstream>

namespace engine::diagnostics
{

std::string_view BuildInfo::CompilerName()
{
    return engine::compiler::Name();
}

std::string_view BuildInfo::PlatformName()
{
    return engine::platform::Name();
}

bool BuildInfo::IsDebugBuild()
{
    return engine::buildconfig::IsDebug;
}

std::string BuildInfo::GetFullBuildInfoString()
{
    std::ostringstream oss;
    oss << "Engine v" << VersionString() << "\n";
    oss << "Build:    " << (IsDebugBuild() ? "Debug" : "Release") << "\n";
    oss << "Compiler: " << CompilerName() << " "
        << engine::compiler::VersionMajor << "."
        << engine::compiler::VersionMinor << "\n";
    oss << "Platform: " << PlatformName() << "\n";
    oss << "Built:    " << BuildDate() << " " << BuildTime();
    return oss.str();
}

} // namespace engine::diagnostics