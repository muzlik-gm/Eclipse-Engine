// ============================================================================
// File: Editor/Include/Editor/Prefs/EditorPreferences.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include <string>
#include <vector>

namespace editor {

    // ========================================================================
    // EditorPreferences — user-specific editor settings.
    // ========================================================================

    /// @brief Stores user-specific editor preferences.  These are
    ///        per-user (not per-project) and persist between sessions.
    struct EditorPreferences
    {
        // -- Theme ---------------------------------------------------------
        std::string ThemeName{"Dark"};

        // -- Camera --------------------------------------------------------
        float CameraMoveSpeed{5.0f};
        float CameraRotateSpeed{0.25f};
        float CameraZoomSpeed{1.0f};
        float CameraPanSpeed{0.01f};
        float CameraNearClip{0.1f};
        float CameraFarClip{1000.0f};
        float CameraFOV{60.0f};

        // -- Grid ----------------------------------------------------------
        bool  GridVisible{true};
        float GridSize{1.0f};
        int   GridDivisions{10};
        engine::math::Vec4 GridColor{0.5f, 0.5f, 0.5f, 0.5f};

        // -- Gizmos --------------------------------------------------------
        int   GizmoMode{0}; // 0=Translate, 1=Rotate, 2=Scale
        bool  GizmoLocalSpace{false};
        float GizmoSnapTranslate{0.5f};
        float GizmoSnapRotate{15.0f};
        float GizmoSnapScale{0.1f};
        bool  GizmoSnapping{false};

        // -- Layout --------------------------------------------------------
        std::string LayoutName{"Default"};

        // -- Recent projects ----------------------------------------------
        std::vector<std::string> RecentProjects;

        // -- Autosave ------------------------------------------------------
        bool   AutosaveEnabled{true};
        float  AutosaveIntervalMinutes{5.0f};

        // -- Console -------------------------------------------------------
        bool   ConsoleAutoScroll{true};
        bool   ConsoleCollapseDuplicates{true};
        int    ConsoleMaxLines{10000};

        // -- Misc ----------------------------------------------------------
        bool   ShowStatisticsOverlay{false};
        bool   VSync{true};

        /// @brief Loads preferences from a JSON file.
        bool Load(const std::string& filePath);

        /// @brief Saves preferences to a JSON file.
        bool Save(const std::string& filePath) const;
    };

} // namespace editor
