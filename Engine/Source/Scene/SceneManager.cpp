// ============================================================================
// File: Engine/Source/Scene/SceneManager.cpp
// ============================================================================
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/Scene.h"

namespace engine::scene {

    std::unique_ptr<Scene> SceneManager::CreateEmpty(const std::string& name) const
    {
        return std::make_unique<Scene>(name);
    }

    void SceneManager::TransitionTo(Scene& scene, SceneState target) const
    {
        SceneState current = GetState(scene);

        switch (target)
        {
            case SceneState::Loaded:
                break;
            case SceneState::Active:
                if (current == SceneState::Loaded || current == SceneState::Inactive)
                    scene.SetActive(true);
                break;
            case SceneState::Inactive:
                if (current == SceneState::Active || current == SceneState::Deactivating)
                    scene.SetActive(false);
                break;
            default:
                break;
        }
    }

    SceneState SceneManager::GetState(const Scene& scene) const
    {
        return scene.IsActive() ? SceneState::Active : SceneState::Loaded;
    }

} // namespace engine::scene
