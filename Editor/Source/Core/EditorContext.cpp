// ============================================================================
// File: Editor/Source/Core/EditorContext.cpp
// ============================================================================
#include "Editor/Core/EditorContext.h"
#include "Editor/Selection/EditorSelection.h"
#include "Editor/Commands/EditorCommandSystem.h"
#include "Editor/Commands/ShortcutManager.h"
#include "Editor/Theme/ThemeManager.h"
#include "Editor/Framework/PanelManager.h"
#include "Editor/Framework/LayoutManager.h"
#include "Editor/Prefs/EditorPreferences.h"
#include "Editor/Prefs/ProjectSettings.h"
#include "Editor/Camera/EditorCamera.h"
#include "Editor/Gizmos/GizmoManager.h"
#include "Editor/Core/EditorIcons.h"
#include "Editor/Core/PlayModeController.h"
#include "Editor/Project/ProjectManager.h"
#include "Editor/Project/SceneManager.h"
#include "Editor/History/CommandHistory.h"

namespace editor {

    EditorContext::EditorContext()
        : m_Selection(std::make_unique<EditorSelection>())
        , m_Commands(std::make_unique<EditorCommandSystem>())
        , m_Shortcuts(std::make_unique<ShortcutManager>(*m_Commands))
        , m_Theme(std::make_unique<ThemeManager>())
        , m_Panels(std::make_unique<PanelManager>())
        , m_Layout(std::make_unique<LayoutManager>())
        , m_Preferences(std::make_unique<EditorPreferences>())
        , m_Project(std::make_unique<ProjectSettings>())
        , m_Camera(std::make_unique<EditorCamera>())
        , m_Gizmos(std::make_unique<GizmoManager>())
        , m_Icons(std::make_unique<EditorIcons>())
        , m_PlayMode(std::make_unique<PlayModeController>())
        , m_ProjectManager(std::make_unique<ProjectManager>())
        , m_SceneManager(std::make_unique<SceneManager>())
        , m_History(std::make_unique<CommandHistory>())
    {
    }

    EditorContext::~EditorContext() = default;

} // namespace editor
