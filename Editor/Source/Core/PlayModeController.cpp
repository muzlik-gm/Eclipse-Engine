// ============================================================================
// File: Editor/Source/Core/PlayModeController.cpp
// ============================================================================
#include "Editor/Core/PlayModeController.h"
#include "Editor/Core/EditorContext.h"
#include "Engine/Core/Log.h"
#include "Engine/Serialization/SceneSerializer.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Entities/EntityHandle.h"

#include <nlohmann/json.hpp>

namespace editor {

    PlayModeController::PlayModeController() = default;

    void PlayModeController::EnterPlayMode(EditorContext& context)
    {
        if (m_Playing)
            return;

        auto* scene = context.GetActiveScene();
        if (!scene)
        {
            ENGINE_LOG_WARN("PlayModeController — no active scene to play");
            return;
        }

        // Serialize the editor scene so we can restore it on stop.
        auto json = engine::serialization::SceneSerializer::Serialize(*scene);
        m_EditorSceneSnapshot = json.dump();

        m_Playing = true;
        m_Paused = false;
        m_Stepping = false;
        m_SimulationTime = 0.0;

        context.SetMode(EditorMode::Play);
        ENGINE_LOG_INFO("PlayModeController — entered play mode");
    }

    void PlayModeController::Pause(EditorContext& context)
    {
        if (!m_Playing || m_Paused)
            return;

        m_Paused = true;
        context.SetMode(EditorMode::Pause);
        ENGINE_LOG_INFO("PlayModeController — paused");
    }

    void PlayModeController::Resume(EditorContext& context)
    {
        if (!m_Playing || !m_Paused)
            return;

        m_Paused = false;
        m_Stepping = false;
        context.SetMode(EditorMode::Play);
        ENGINE_LOG_INFO("PlayModeController — resumed");
    }

    void PlayModeController::Stop(EditorContext& context)
    {
        if (!m_Playing)
            return;

        // Restore the editor scene from the snapshot.
        auto* scene = context.GetActiveScene();
        if (scene && !m_EditorSceneSnapshot.empty())
        {
            try
            {
                auto json = nlohmann::json::parse(m_EditorSceneSnapshot);
                auto restored = engine::serialization::SceneSerializer::Deserialize(json);

                // Copy the restored scene's registry back into the active scene.
                // Since we can't directly replace the registry, we clear and
                // re-populate.  For simplicity, we swap the active scene pointer.
                // In production, Scene would support a RestoreFrom() method.
                auto& oldRegistry = scene->GetRegistry();
                oldRegistry.Clear();

                // Re-create entities from the restored scene.
                auto& newRegistry = restored->GetRegistry();
                auto view = newRegistry.View<engine::components::TagComponent>();
                for (auto entity : view)
                {
                    auto handle = scene->CreateEntity(
                        newRegistry.GetComponent<engine::components::TagComponent>(entity).Tag);

                    // Copy IDComponent.
                    if (newRegistry.HasComponent<engine::components::IDComponent>(entity))
                    {
                        auto& id = newRegistry.GetComponent<engine::components::IDComponent>(entity);
                        handle.GetComponent<engine::components::IDComponent>().ID = id.ID;
                    }

                    // Copy TransformComponent.
                    if (newRegistry.HasComponent<engine::components::TransformComponent>(entity))
                    {
                        auto& tf = newRegistry.GetComponent<engine::components::TransformComponent>(entity);
                        handle.AddComponent<engine::components::TransformComponent>(tf);
                    }

                    // Copy other components.
                    if (newRegistry.HasComponent<engine::components::MeshComponent>(entity))
                        handle.AddComponent<engine::components::MeshComponent>(
                            newRegistry.GetComponent<engine::components::MeshComponent>(entity));
                    if (newRegistry.HasComponent<engine::components::CameraComponent>(entity))
                        handle.AddComponent<engine::components::CameraComponent>(
                            newRegistry.GetComponent<engine::components::CameraComponent>(entity));
                    if (newRegistry.HasComponent<engine::components::LightComponent>(entity))
                        handle.AddComponent<engine::components::LightComponent>(
                            newRegistry.GetComponent<engine::components::LightComponent>(entity));
                    if (newRegistry.HasComponent<engine::components::VisibilityComponent>(entity))
                        handle.AddComponent<engine::components::VisibilityComponent>(
                            newRegistry.GetComponent<engine::components::VisibilityComponent>(entity));
                }
            }
            catch (const std::exception& e)
            {
                ENGINE_LOG_ERROR("PlayModeController — restore failed: {}", e.what());
            }
        }

        m_Playing = false;
        m_Paused = false;
        m_Stepping = false;
        m_SimulationTime = 0.0;
        m_EditorSceneSnapshot.clear();

        context.SetMode(EditorMode::Edit);
        ENGINE_LOG_INFO("PlayModeController — stopped, editor scene restored");
    }

    void PlayModeController::Step(EditorContext& context)
    {
        if (!m_Playing || !m_Paused)
            return;

        m_Stepping = true;
        context.SetMode(EditorMode::Step);
        ENGINE_LOG_DEBUG("PlayModeController — stepping one frame");
    }

} // namespace editor
