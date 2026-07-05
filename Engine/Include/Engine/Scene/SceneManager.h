// ============================================================================
// File: Engine/Include/Engine/Scene/SceneManager.h
// Utility class for scene loading, saving, and lifecycle transitions.
// ============================================================================
#pragma once

#include "Engine/Scene/SceneLifecycle.h"
#include "Engine/Core/UUID.h"

#include <memory>
#include <string>

namespace engine::scene {

    class Scene;

    /// @brief Utility for scene creation and lifecycle management.
    class SceneManager
    {
    public:
        SceneManager()  = default;
        ~SceneManager() = default;

        [[nodiscard]] std::unique_ptr<Scene> CreateEmpty(const std::string& name) const;
        void TransitionTo(Scene& scene, SceneState target) const;
        [[nodiscard]] SceneState GetState(const Scene& scene) const;

        [[nodiscard]] static bool CanUpdate(SceneState state)
        {
            return state == SceneState::Active;
        }
    };

} // namespace engine::scene
