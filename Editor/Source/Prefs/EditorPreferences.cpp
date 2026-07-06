// ============================================================================
// File: Editor/Source/Prefs/EditorPreferences.cpp
// ============================================================================
#include "Editor/Prefs/EditorPreferences.h"
#include "Engine/Core/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace editor {

    namespace fs = std::filesystem;
    using json = nlohmann::json;

    bool EditorPreferences::Load(const std::string& filePath)
    {
        try
        {
            std::ifstream ifs(filePath);
            if (!ifs.is_open())
                return false;

            json j;
            ifs >> j;

            ThemeName     = j.value("theme", "Dark");
            CameraMoveSpeed   = j.value("camera_move_speed", 5.0f);
            CameraRotateSpeed = j.value("camera_rotate_speed", 0.25f);
            CameraZoomSpeed   = j.value("camera_zoom_speed", 1.0f);
            CameraPanSpeed    = j.value("camera_pan_speed", 0.01f);
            CameraNearClip    = j.value("camera_near_clip", 0.1f);
            CameraFarClip     = j.value("camera_far_clip", 1000.0f);
            CameraFOV         = j.value("camera_fov", 60.0f);

            GridVisible   = j.value("grid_visible", true);
            GridSize      = j.value("grid_size", 1.0f);
            GridDivisions = j.value("grid_divisions", 10);

            GizmoMode         = j.value("gizmo_mode", 0);
            GizmoLocalSpace   = j.value("gizmo_local_space", false);
            GizmoSnapTranslate = j.value("gizmo_snap_translate", 0.5f);
            GizmoSnapRotate    = j.value("gizmo_snap_rotate", 15.0f);
            GizmoSnapScale     = j.value("gizmo_snap_scale", 0.1f);
            GizmoSnapping      = j.value("gizmo_snapping", false);

            LayoutName = j.value("layout_name", "Default");

            RecentProjects.clear();
            if (j.contains("recent_projects"))
            {
                for (const auto& p : j["recent_projects"])
                    RecentProjects.push_back(p.get<std::string>());
            }

            AutosaveEnabled = j.value("autosave_enabled", true);
            AutosaveIntervalMinutes = j.value("autosave_interval", 5.0f);

            ConsoleAutoScroll = j.value("console_auto_scroll", true);
            ConsoleCollapseDuplicates = j.value("console_collapse_duplicates", true);
            ConsoleMaxLines = j.value("console_max_lines", 10000);

            ShowStatisticsOverlay = j.value("show_statistics_overlay", false);
            VSync = j.value("vsync", true);

            ENGINE_LOG_INFO("EditorPreferences — loaded from '{}'", filePath);
            return true;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("EditorPreferences — load failed: {}", e.what());
            return false;
        }
    }

    bool EditorPreferences::Save(const std::string& filePath) const
    {
        try
        {
            fs::create_directories(fs::path(filePath).parent_path());

            json j;
            j["theme"] = ThemeName;
            j["camera_move_speed"] = CameraMoveSpeed;
            j["camera_rotate_speed"] = CameraRotateSpeed;
            j["camera_zoom_speed"] = CameraZoomSpeed;
            j["camera_pan_speed"] = CameraPanSpeed;
            j["camera_near_clip"] = CameraNearClip;
            j["camera_far_clip"] = CameraFarClip;
            j["camera_fov"] = CameraFOV;

            j["grid_visible"] = GridVisible;
            j["grid_size"] = GridSize;
            j["grid_divisions"] = GridDivisions;

            j["gizmo_mode"] = GizmoMode;
            j["gizmo_local_space"] = GizmoLocalSpace;
            j["gizmo_snap_translate"] = GizmoSnapTranslate;
            j["gizmo_snap_rotate"] = GizmoSnapRotate;
            j["gizmo_snap_scale"] = GizmoSnapScale;
            j["gizmo_snapping"] = GizmoSnapping;

            j["layout_name"] = LayoutName;

            json recent = json::array();
            for (const auto& p : RecentProjects)
                recent.push_back(p);
            j["recent_projects"] = recent;

            j["autosave_enabled"] = AutosaveEnabled;
            j["autosave_interval"] = AutosaveIntervalMinutes;

            j["console_auto_scroll"] = ConsoleAutoScroll;
            j["console_collapse_duplicates"] = ConsoleCollapseDuplicates;
            j["console_max_lines"] = ConsoleMaxLines;

            j["show_statistics_overlay"] = ShowStatisticsOverlay;
            j["vsync"] = VSync;

            std::ofstream ofs(filePath);
            if (!ofs.is_open())
            {
                ENGINE_LOG_ERROR("EditorPreferences — cannot open '{}'", filePath);
                return false;
            }
            ofs << j.dump(4);
            return true;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("EditorPreferences — save failed: {}", e.what());
            return false;
        }
    }

} // namespace editor
