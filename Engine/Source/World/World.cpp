// ============================================================================
// File: Engine/Source/World/World.cpp
// World subsystem implementation.
// ============================================================================

#include "Engine/World/World.h"
#include "Engine/Core/Log.h"

namespace engine::world {

    // ========================================================================
    // ISubsystem interface
    // ========================================================================

    std::string_view World::GetName() const noexcept
    {
        return "World";
    }

    std::vector<std::string> World::GetDependencies() const
    {
        return {}; // No subsystem dependencies.
    }

    bool World::Initialize()
    {
        ENGINE_LOG_INFO("World — initializing");

        // Create a default "Main Scene" and make it active.
        CreateScene("Main Scene");

        if (m_EventBus)
        {
            world_events::WorldInitializedEvent event;
            m_EventBus->Dispatch(event);
        }

        return true;
    }

    void World::Shutdown()
    {
        ENGINE_LOG_INFO("World — shutting down");

        if (m_EventBus)
        {
            world_events::WorldShutdownEvent event;
            m_EventBus->Dispatch(event);
        }

        // Detach all systems and destroy all scenes.
        m_scenes.clear();
        m_activeScene = nullptr;
    }

    void World::Update(f64 deltaTime)
    {
        if (m_activeScene && m_activeScene->IsActive())
        {
            if (m_EventBus)
            {
                world_events::WorldTickEvent event(deltaTime);
                m_EventBus->Dispatch(event);
            }

            m_activeScene->OnUpdate(deltaTime);
            m_activeScene->OnLateUpdate(deltaTime);
        }
    }

    void World::FixedUpdate(f64 fixedDeltaTime)
    {
        if (m_activeScene && m_activeScene->IsActive())
        {
            m_activeScene->OnFixedUpdate(fixedDeltaTime);
        }
    }

    void World::LateUpdate([[maybe_unused]] f64 deltaTime)
    {
        // LateUpdate is already called at the end of Update() so the
        // scheduler runs Update → LateUpdate in the correct order within
        // a single scene tick.  This method is a no-op to satisfy the
        // ISubsystem interface.
    }

    // ========================================================================
    // Scene management
    // ========================================================================

    scene::Scene& World::CreateScene(const std::string& name)
    {
        auto scene = std::make_unique<scene::Scene>(name, m_EventBus);
        scene::Scene* raw = scene.get();

        const auto& uuid = scene->GetUUID();
        m_scenes[uuid] = std::move(scene);

        // If this is the very first scene, make it active automatically.
        if (m_activeScene == nullptr)
        {
            m_activeScene = raw;
            m_activeScene->SetActive(true);

            if (m_EventBus)
            {
                scene_events::SceneActivatedEvent event(uuid);
                m_EventBus->Dispatch(event);
            }
        }

        ENGINE_LOG_DEBUG("World — created scene '{}' ({})", name, uuid.ToString());
        return *raw;
    }

    void World::DestroyScene(const core::UUID& uuid)
    {
        auto it = m_scenes.find(uuid);
        if (it == m_scenes.end())
            return;

        ENGINE_LOG_DEBUG("World — destroying scene {}", uuid.ToString());

        if (m_EventBus)
        {
            scene_events::SceneUnloadedEvent event(uuid);
            m_EventBus->Dispatch(event);
        }

        // If the destroyed scene was active, select a new active scene.
        if (m_activeScene == it->second.get())
        {
            m_activeScene = nullptr;

            // Pick the first remaining scene.
            for (auto& [id, s] : m_scenes)
            {
                if (id != uuid)
                {
                    m_activeScene = s.get();
                    m_activeScene->SetActive(true);

                    if (m_EventBus)
                    {
                        scene_events::SceneActivatedEvent activateEvent(id);
                        m_EventBus->Dispatch(activateEvent);
                    }
                    break;
                }
            }
        }

        m_scenes.erase(it);
    }

    scene::Scene* World::GetScene(const core::UUID& uuid)
    {
        auto it = m_scenes.find(uuid);
        if (it == m_scenes.end())
            return nullptr;
        return it->second.get();
    }

    void World::SetActiveScene(const core::UUID& uuid)
    {
        auto* scene = GetScene(uuid);
        if (!scene)
            return;

        core::UUID oldUUID{};
        if (m_activeScene)
        {
            oldUUID = m_activeScene->GetUUID();
            m_activeScene->SetActive(false);
        }

        m_activeScene = scene;
        m_activeScene->SetActive(true);

        if (m_EventBus)
        {
            world_events::ActiveSceneChangedEvent event(oldUUID, uuid);
            m_EventBus->Dispatch(event);

            scene_events::SceneActivatedEvent activateEvent(uuid);
            m_EventBus->Dispatch(activateEvent);
        }

        ENGINE_LOG_DEBUG("World — active scene changed: {} -> {}",
                         oldUUID.ToString(), uuid.ToString());
    }

} // namespace engine::world
