// ============================================================================
// File: Editor/Source/Core/MenuBar.cpp
// ============================================================================
#include "Editor/Core/MenuBar.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Commands/EditorCommandSystem.h"
#include "Editor/Commands/ShortcutManager.h"
#include "Editor/Framework/PanelManager.h"

#include <imgui.h>

namespace editor {

    void MenuBar::Render(EditorContext& context)
    {
        if (!ImGui::BeginMenuBar())
            return;

        RenderFileMenu(context);
        RenderEditMenu(context);
        RenderViewMenu(context);
        RenderWindowMenu(context);
        RenderToolsMenu(context);
        RenderHelpMenu(context);

        ImGui::EndMenuBar();
    }

    void MenuBar::MenuItem(EditorContext& context, const char* label, const char* commandName)
    {
        auto* cmd = context.GetCommands().Find(commandName);
        bool enabled = cmd ? context.GetCommands().CanExecute(commandName) : false;

        // Look up shortcut display.
        std::string shortcut;
        auto* sc = context.GetShortcuts().FindByCommand(commandName);
        if (sc)
            shortcut = sc->DisplayName;

        if (ImGui::MenuItem(label, shortcut.empty() ? nullptr : shortcut.c_str(), false, enabled))
        {
            if (enabled)
                context.GetCommands().Execute(commandName);
        }
    }

    void MenuBar::RenderFileMenu(EditorContext& context)
    {
        if (!ImGui::BeginMenu("File"))
            return;

        MenuItem(context, "New Project", "file.new_project");
        MenuItem(context, "Open Project...", "file.open_project");
        MenuItem(context, "Save Project", "file.save_project");
        MenuItem(context, "Save Project As...", "file.save_project_as");
        ImGui::Separator();
        MenuItem(context, "New Scene", "file.new_scene");
        MenuItem(context, "Open Scene...", "file.open_scene");
        MenuItem(context, "Save Scene", "file.save_scene");
        MenuItem(context, "Save Scene As...", "file.save_scene_as");
        ImGui::Separator();
        MenuItem(context, "Exit", "file.exit");

        ImGui::EndMenu();
    }

    void MenuBar::RenderEditMenu(EditorContext& context)
    {
        if (!ImGui::BeginMenu("Edit"))
            return;

        MenuItem(context, "Undo", "edit.undo");
        MenuItem(context, "Redo", "edit.redo");
        ImGui::Separator();
        MenuItem(context, "Cut", "edit.cut");
        MenuItem(context, "Copy", "edit.copy");
        MenuItem(context, "Paste", "edit.paste");
        MenuItem(context, "Duplicate", "edit.duplicate");
        MenuItem(context, "Delete", "edit.delete");
        ImGui::Separator();
        MenuItem(context, "Preferences...", "edit.preferences");

        ImGui::EndMenu();
    }

    void MenuBar::RenderViewMenu(EditorContext& context)
    {
        if (!ImGui::BeginMenu("View"))
            return;

        MenuItem(context, "Toggle Grid", "view.toggle_grid");
        MenuItem(context, "Toggle Gizmos", "view.toggle_gizmos");
        MenuItem(context, "Frame Selected (F)", "view.frame_selected");
        ImGui::Separator();
        MenuItem(context, "Zoom In", "view.zoom_in");
        MenuItem(context, "Zoom Out", "view.zoom_out");
        MenuItem(context, "Reset Camera", "view.reset_camera");

        ImGui::EndMenu();
    }

    void MenuBar::RenderWindowMenu(EditorContext& context)
    {
        if (!ImGui::BeginMenu("Window"))
            return;

        // List all panels with a checkmark if open.
        for (auto* panel : context.GetPanels().GetAllPanels())
        {
            bool open = panel->IsOpen();
            if (ImGui::MenuItem(panel->GetTitle().c_str(), nullptr, &open))
            {
                panel->SetOpen(open);
            }
        }

        ImGui::Separator();
        MenuItem(context, "Close All Panels", "window.close_all");
        MenuItem(context, "Reset Layout", "window.reset_layout");

        ImGui::EndMenu();
    }

    void MenuBar::RenderToolsMenu(EditorContext& context)
    {
        if (!ImGui::BeginMenu("Tools"))
            return;

        MenuItem(context, "Build Asset Database", "tools.build_assets");
        MenuItem(context, "Reimport All", "tools.reimport_all");
        ImGui::Separator();
        MenuItem(context, "Project Settings...", "tools.project_settings");
        MenuItem(context, "Editor Preferences...", "tools.editor_preferences");

        ImGui::EndMenu();
    }

    void MenuBar::RenderHelpMenu(EditorContext& context)
    {
        if (!ImGui::BeginMenu("Help"))
            return;

        MenuItem(context, "Documentation", "help.documentation");
        MenuItem(context, "About", "help.about");

        ImGui::EndMenu();
    }

} // namespace editor
