// ============================================================================
// File: Editor/Include/Editor/Core/EditorContext.h
// Central context holding all shared editor state.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Events/EventBus.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Core/Renderer.h"

#include <memory>
#include <string>

namespace editor {

    class EditorSelection;
    class EditorCommandSystem;
    class ShortcutManager;
    class ThemeManager;
    class PanelManager;
    class LayoutManager;
    class EditorPreferences;
    class ProjectSettings;
    class EditorCamera;
    class GizmoManager;
    class EditorIcons;
    class PlayModeController;
    class ProjectManager;
    class SceneManager;
    class CommandHistory;

    namespace runtime = engine::runtime;
    namespace events = engine::events;
    namespace scene = engine::scene;
    namespace renderer = engine::renderer;

    // ========================================================================
    // EditorMode — the current editor play mode.
    // ========================================================================

    enum class EditorMode : engine::core::u32
    {
        Edit    = 0,
        Play    = 1,
        Pause   = 2,
        Step    = 3
    };

    // ========================================================================
    // EditorContext — holds all shared editor state.
    // ========================================================================

    /// @brief The EditorContext is the central hub that every editor
    ///        subsystem references.  It is created once by the
    ///        EditorApplication and passed to all panels, commands, and
    ///        framework systems.
    class EditorContext
    {
    public:
        EditorContext();
        ~EditorContext();

        EditorContext(const EditorContext&)            = delete;
        EditorContext& operator=(const EditorContext&) = delete;
        EditorContext(EditorContext&&)                 = delete;
        EditorContext& operator=(EditorContext&&)      = delete;

        // -- Subsystem access ----------------------------------------------

        [[nodiscard]] EditorSelection&     GetSelection()     noexcept { return *m_Selection; }
        [[nodiscard]] EditorCommandSystem& GetCommands()      noexcept { return *m_Commands; }
        [[nodiscard]] ShortcutManager&     GetShortcuts()     noexcept { return *m_Shortcuts; }
        [[nodiscard]] ThemeManager&        GetTheme()         noexcept { return *m_Theme; }
        [[nodiscard]] PanelManager&        GetPanels()        noexcept { return *m_Panels; }
        [[nodiscard]] LayoutManager&       GetLayout()        noexcept { return *m_Layout; }
        [[nodiscard]] EditorPreferences&   GetPreferences()   noexcept { return *m_Preferences; }
        [[nodiscard]] ProjectSettings&     GetProject()       noexcept { return *m_Project; }
        [[nodiscard]] EditorCamera&        GetCamera()        noexcept { return *m_Camera; }
        [[nodiscard]] GizmoManager&        GetGizmos()        noexcept { return *m_Gizmos; }
        [[nodiscard]] EditorIcons&         GetIcons()         noexcept { return *m_Icons; }
        [[nodiscard]] PlayModeController&  GetPlayMode()      noexcept { return *m_PlayMode; }
        [[nodiscard]] ProjectManager&      GetProjectManager() noexcept { return *m_ProjectManager; }
        [[nodiscard]] SceneManager&        GetSceneManager()   noexcept { return *m_SceneManager; }
        [[nodiscard]] CommandHistory&      GetHistory()        noexcept { return *m_History; }

        // -- Engine access -------------------------------------------------

        [[nodiscard]] events::EventBus&    GetEventBus()      noexcept { return *m_EventBus; }
        [[nodiscard]] scene::Scene*        GetActiveScene()   noexcept { return m_ActiveScene; }
        [[nodiscard]] renderer::Renderer*  GetRenderer()      noexcept { return m_Renderer; }

        void SetEventBus(events::EventBus* bus) noexcept { m_EventBus = bus; }
        void SetActiveScene(scene::Scene* scene) noexcept { m_ActiveScene = scene; }
        void SetRenderer(renderer::Renderer* renderer) noexcept { m_Renderer = renderer; }

        // -- Play mode -----------------------------------------------------

        [[nodiscard]] EditorMode GetMode() const noexcept { return m_Mode; }
        void SetMode(EditorMode mode) noexcept { m_Mode = mode; }
        [[nodiscard]] bool IsPlaying() const noexcept { return m_Mode == EditorMode::Play; }
        [[nodiscard]] bool IsPaused() const noexcept { return m_Mode == EditorMode::Pause; }

        // -- Frame info ----------------------------------------------------

        void SetDeltaTime(engine::core::f64 dt) noexcept { m_DeltaTime = dt; }
        [[nodiscard]] engine::core::f64 GetDeltaTime() const noexcept { return m_DeltaTime; }

    private:
        std::unique_ptr<EditorSelection>     m_Selection;
        std::unique_ptr<EditorCommandSystem> m_Commands;
        std::unique_ptr<ShortcutManager>     m_Shortcuts;
        std::unique_ptr<ThemeManager>        m_Theme;
        std::unique_ptr<PanelManager>        m_Panels;
        std::unique_ptr<LayoutManager>       m_Layout;
        std::unique_ptr<EditorPreferences>   m_Preferences;
        std::unique_ptr<ProjectSettings>     m_Project;
        std::unique_ptr<EditorCamera>        m_Camera;
        std::unique_ptr<GizmoManager>        m_Gizmos;
        std::unique_ptr<EditorIcons>         m_Icons;
        std::unique_ptr<PlayModeController>  m_PlayMode;
        std::unique_ptr<ProjectManager>      m_ProjectManager;
        std::unique_ptr<SceneManager>        m_SceneManager;
        std::unique_ptr<CommandHistory>      m_History;

        events::EventBus*                    m_EventBus{nullptr};
        scene::Scene*                        m_ActiveScene{nullptr};
        renderer::Renderer*                  m_Renderer{nullptr};

        EditorMode                           m_Mode{EditorMode::Edit};
        engine::core::f64                    m_DeltaTime{0.0};
    };

} // namespace editor
