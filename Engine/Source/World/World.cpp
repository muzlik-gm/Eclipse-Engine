// ============================================================================
// File: Engine/Source/World/World.cpp
// World subsystem implementation.
// ============================================================================

#include "Engine/World/World.h"

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
        // Create a default "Main Scene" and make it active.
        CreateScene("Main Scene");
        return true;
    }

    void World::Shutdown()
    {
        // Detach all systems and destroy all scenes.
        m_scenes.clear();
        m_activeScene = nullptr;
    }

    void World::Update(f64 deltaTime)
    {
        if (m_activeScene && m_activeScene->IsActive())
        {
            m_activeScene->OnUpdate(deltaTime);
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
        // No-op for now — reserved for future use.
    }

    // ========================================================================
    // Scene management
    // ========================================================================

    scene::Scene& World::CreateScene(const std::string& name)
    {
        auto scene = std::make_unique<scene::Scene>(name);
        scene::Scene* raw = scene.get();

        const auto& uuid = scene->GetUUID();
        m_scenes[uuid] = std::move(scene);

        // If this is the very first scene, make it active automatically.
        if (m_activeScene == nullptr)
        {
            m_activeScene = raw;
            m_activeScene->SetActive(true);
        }

        return *raw;
    }

    void World::DestroyScene(const core::UUID& uuid)
    {
        auto it = m_scenes.find(uuid);
        if (it == m_scenes.end())
            return;

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

        // Deactivate the previous active scene.
        if (m_activeScene)
        {
            m_activeScene->SetActive(false);
        }

        m_activeScene = scene;
        m_activeScene->SetActive(true);
    }

} // namespace engine::world