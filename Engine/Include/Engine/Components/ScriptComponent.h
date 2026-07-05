// ============================================================================
// File: Engine/Include/Engine/Components/ScriptComponent.h
// Runtime representation of an entity script attachment.
// ============================================================================
#pragma once

#include <string>

namespace engine::components {

    /// @brief Attaches a script to an entity.
    struct ScriptComponent
    {
        std::string ScriptPath;
        bool        IsEnabled{true};
    };

} // namespace engine::components
