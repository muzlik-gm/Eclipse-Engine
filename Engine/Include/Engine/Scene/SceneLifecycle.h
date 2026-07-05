// ============================================================================
// File: Engine/Include/Engine/Scene/SceneLifecycle.h
// Explicit lifecycle states for a Scene.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace engine::scene {

    using engine::core::u32;

    enum class SceneState : u32
    {
        Unloaded    = 0,
        Loading     = 1,
        Loaded      = 2,
        Activating  = 3,
        Active      = 4,
        Deactivating = 5,
        Inactive    = 6,
        Unloading   = 7,
        Destroyed   = 8
    };

    [[nodiscard]] constexpr const char* SceneStateToString(SceneState state) noexcept
    {
        switch (state)
        {
            case SceneState::Unloaded:     return "Unloaded";
            case SceneState::Loading:      return "Loading";
            case SceneState::Loaded:       return "Loaded";
            case SceneState::Activating:   return "Activating";
            case SceneState::Active:       return "Active";
            case SceneState::Deactivating: return "Deactivating";
            case SceneState::Inactive:     return "Inactive";
            case SceneState::Unloading:    return "Unloading";
            case SceneState::Destroyed:    return "Destroyed";
            default:                       return "Unknown";
        }
    }

} // namespace engine::scene
