// ============================================================================
// File: Editor/Source/Project/SceneManager.cpp
// ============================================================================
#include "Editor/Project/SceneManager.h"
#include "Engine/Core/Log.h"
#include "Engine/Serialization/SceneSerializer.h"

#include <algorithm>

namespace editor {

    SceneManager::SceneManager() = default;
    SceneManager::~SceneManager() = default;

    // ========================================================================
    // Scene lifecycle
    // ========================================================================

    std::shared_ptr<engine::scene::Scene> SceneManager::NewScene(const std::string& name)
    {
        m_Current = std::make_shared<engine::scene::Scene>(name);
        m_CurrentScenePath.clear();
        m_Dirty = false;
        NotifyChanged();
        ENGINE_LOG_INFO("SceneManager — new scene '{}'", name);
        return m_Current;
    }

    std::shared_ptr<engine::scene::Scene> SceneManager::LoadScene(const std::string& filePath)
    {
        auto scene = engine::serialization::SceneSerializer::LoadFromFile(filePath);
        if (!scene)
        {
            ENGINE_LOG_ERROR("SceneManager — failed to load scene '{}'", filePath);
            return nullptr;
        }

        m_Current = std::shared_ptr<engine::scene::Scene>(scene.release());
        m_CurrentScenePath = filePath;
        m_Dirty = false;

        AddRecentScene(filePath);
        NotifyChanged();
        ENGINE_LOG_INFO("SceneManager — loaded scene from '{}'", filePath);
        return m_Current;
    }

    bool SceneManager::SaveScene()
    {
        if (!m_Current || m_CurrentScenePath.empty())
            return false;

        bool success = engine::serialization::SceneSerializer::SaveToFile(
            *m_Current, m_CurrentScenePath);

        if (success)
        {
            m_Dirty = false;
            ENGINE_LOG_INFO("SceneManager — saved scene to '{}'", m_CurrentScenePath);
        }
        return success;
    }

    bool SceneManager::SaveSceneAs(const std::string& filePath)
    {
        if (!m_Current)
            return false;

        bool success = engine::serialization::SceneSerializer::SaveToFile(
            *m_Current, filePath);

        if (success)
        {
            m_CurrentScenePath = filePath;
            m_Dirty = false;
            AddRecentScene(filePath);
            ENGINE_LOG_INFO("SceneManager — saved scene as '{}'", filePath);
        }
        return success;
    }

    void SceneManager::CloseScene()
    {
        m_Current.reset();
        m_CurrentScenePath.clear();
        m_Dirty = false;
        NotifyChanged();
        ENGINE_LOG_INFO("SceneManager — scene closed");
    }

    bool SceneManager::ReloadScene()
    {
        if (m_CurrentScenePath.empty())
            return false;

        auto path = m_CurrentScenePath;
        CloseScene();
        return LoadScene(path) != nullptr;
    }

    // ========================================================================
    // Recent scenes
    // ========================================================================

    void SceneManager::AddRecentScene(const std::string& filePath)
    {
        m_RecentScenes.erase(
            std::remove(m_RecentScenes.begin(), m_RecentScenes.end(), filePath),
            m_RecentScenes.end());
        m_RecentScenes.insert(m_RecentScenes.begin(), filePath);
        if (m_RecentScenes.size() > 10)
            m_RecentScenes.resize(10);
    }

    // ========================================================================
    // Callbacks
    // ========================================================================

    engine::core::u32 SceneManager::Subscribe(SceneChangedCallback callback)
    {
        auto id = m_NextId++;
        m_Callbacks[id] = std::move(callback);
        return id;
    }

    void SceneManager::Unsubscribe(engine::core::u32 id)
    {
        m_Callbacks.erase(id);
    }

    void SceneManager::NotifyChanged()
    {
        for (const auto& [_, cb] : m_Callbacks)
        {
            if (cb)
                cb(m_Current.get());
        }
    }

} // namespace editor
