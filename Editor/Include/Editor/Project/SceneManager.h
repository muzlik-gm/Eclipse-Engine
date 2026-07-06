// ============================================================================
// File: Editor/Include/Editor/Project/SceneManager.h
// Scene lifecycle management — new/save/load/close with dirty state.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Scene/Scene.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace editor {

    class Project;

    // ========================================================================
    // SceneManager — manages scene lifecycle in the editor.
    // ========================================================================

    /// @brief Manages scene creation, saving, loading, and closing.
    ///        Tracks the dirty state so the editor can prompt for
    ///        unsaved changes.
    class SceneManager
    {
    public:
        SceneManager();
        ~SceneManager();

        /// @brief Creates a new empty scene.
        std::shared_ptr<engine::scene::Scene> NewScene(const std::string& name = "New Scene");

        /// @brief Loads a scene from a .scene file.
        std::shared_ptr<engine::scene::Scene> LoadScene(const std::string& filePath);

        /// @brief Saves the current scene to its file path.
        bool SaveScene();

        /// @brief Saves the current scene to a new file path.
        bool SaveSceneAs(const std::string& filePath);

        /// @brief Closes the current scene.
        void CloseScene();

        /// @brief Reloads the current scene from disk.
        bool ReloadScene();

        // -- Accessors -----------------------------------------------------

        [[nodiscard]] engine::scene::Scene* GetCurrentScene() const noexcept
        { return m_Current.get(); }

        [[nodiscard]] std::shared_ptr<engine::scene::Scene> GetCurrentScenePtr() const noexcept
        { return m_Current; }

        [[nodiscard]] const std::string& GetCurrentScenePath() const noexcept
        { return m_CurrentScenePath; }

        [[nodiscard]] bool IsDirty() const noexcept { return m_Dirty; }
        void SetDirty(bool dirty) noexcept { m_Dirty = dirty; }

        [[nodiscard]] bool HasScene() const noexcept { return m_Current != nullptr; }

        // -- Recent scenes -------------------------------------------------

        [[nodiscard]] const std::vector<std::string>& GetRecentScenes() const noexcept
        { return m_RecentScenes; }

        void AddRecentScene(const std::string& filePath);

        // -- Callbacks -----------------------------------------------------

        using SceneChangedCallback = std::function<void(engine::scene::Scene*)>;
        engine::core::u32 Subscribe(SceneChangedCallback callback);
        void Unsubscribe(engine::core::u32 id);

    private:
        void NotifyChanged();

        std::shared_ptr<engine::scene::Scene>    m_Current;
        std::string                               m_CurrentScenePath;
        bool                                      m_Dirty{false};
        std::vector<std::string>                  m_RecentScenes;

        std::unordered_map<engine::core::u32,
            SceneChangedCallback>                 m_Callbacks;
        engine::core::u32                         m_NextId{1};
    };

} // namespace editor
