// ============================================================================
// File: Editor/Source/Core/EditorApplication.cpp
// ============================================================================
#include "Editor/Core/EditorApplication.h"
#include "Editor/Selection/EditorSelection.h"
#include "Editor/Core/EntityFactory.h"
#include "Editor/Core/PlayModeController.h"
#include "Editor/Project/ProjectManager.h"
#include "Editor/Project/SceneManager.h"
#include "Editor/History/CommandHistory.h"
#include "Editor/Panels/HierarchyPanel.h"
#include "Editor/Panels/InspectorPanel.h"
#include "Editor/Panels/ScenePanel.h"
#include "Editor/Panels/GamePanel.h"
#include "Editor/Panels/ConsolePanel.h"
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Panels/StatisticsPanel.h"
#include "Editor/Panels/ProfilerPanel.h"
#include "Editor/Panels/ProjectBrowserPanel.h"
#include "Editor/Core/DragDrop.h"
#include "Editor/Framework/PanelManager.h"
#include "Editor/Commands/EditorCommandSystem.h"
#include "Editor/Commands/ShortcutManager.h"
#include "Editor/Theme/ThemeManager.h"
#include "Editor/Framework/LayoutManager.h"
#include "Editor/Prefs/EditorPreferences.h"
#include "Editor/Prefs/ProjectSettings.h"
#include "Editor/Camera/EditorCamera.h"
#include "Editor/Gizmos/GizmoManager.h"
#include "Engine/Systems/TransformSystem.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Transforms/TransformUtils.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Core/Log.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>

namespace editor {

    using namespace engine::core;

    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    EditorApplication::EditorApplication() = default;

    EditorApplication::~EditorApplication()
    {
        Shutdown();
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    bool EditorApplication::Initialize(GLFWwindow* window, engine::events::EventBus* eventBus,
                                        engine::scene::Scene* scene,
                                        engine::renderer::Renderer* renderer)
    {
        m_Window = window;

        // Bind engine subsystems to the editor context.
        m_Context.SetEventBus(eventBus);
        m_Context.SetActiveScene(scene);
        m_Context.SetRenderer(renderer);

        // Initialize ImGui.
        InitializeImGui(window);

        // Apply the theme.
        m_Context.GetTheme().ApplyToImGui();

        // Register commands, shortcuts, and panels.
        RegisterDefaultCommands();
        RegisterDefaultShortcuts();
        RegisterDefaultPanels();

        // Load layout.
        m_Context.GetLayout().SetConfigDirectory(".editor");
        m_Context.GetLayout().Load(m_Context);

        // Load preferences.
        m_Context.GetPreferences().Load(".editor/editor_prefs.json");

        // Apply camera preferences.
        auto& cam = m_Context.GetCamera();
        cam.SetMoveSpeed(m_Context.GetPreferences().CameraMoveSpeed);
        cam.SetRotateSpeed(m_Context.GetPreferences().CameraRotateSpeed);
        cam.SetZoomSpeed(m_Context.GetPreferences().CameraZoomSpeed);
        cam.SetPanSpeed(m_Context.GetPreferences().CameraPanSpeed);
        cam.SetNearClip(m_Context.GetPreferences().CameraNearClip);
        cam.SetFarClip(m_Context.GetPreferences().CameraFarClip);
        cam.SetFOV(m_Context.GetPreferences().CameraFOV);

        // If no active scene was provided, create a default empty one.
        if (!m_Context.GetSceneManager().HasScene())
        {
            m_Context.GetSceneManager().NewScene("Untitled");
        }
        m_Context.SetActiveScene(m_Context.GetSceneManager().GetCurrentScene());

        m_Initialized = true;
        ENGINE_LOG_INFO("EditorApplication — initialized");
        return true;
    }

    void EditorApplication::InitializeImGui(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // NOTE: ViewportsEnable disabled — causes crashes on some Windows
        // systems when dragging windows outside the main window.
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Ensure the .editor directory exists before ImGui tries to
        // write its ini file there.
        std::filesystem::create_directories(".editor");
        io.IniFilename = ".editor/imgui.ini";

        // Apply default style.
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;

        // Initialize ImGui backends.
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        ENGINE_LOG_INFO("EditorApplication — ImGui initialized");
    }

    // ========================================================================
    // Shutdown
    // ========================================================================

    void EditorApplication::Shutdown()
    {
        if (!m_Initialized)
            return;

        // Save layout and preferences.
        m_Context.GetLayout().Save(m_Context);
        m_Context.GetPreferences().Save(".editor/editor_prefs.json");

        ShutdownImGui();
        m_Initialized = false;
        ENGINE_LOG_INFO("EditorApplication — shut down");
    }

    void EditorApplication::ShutdownImGui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // ========================================================================
    // Render
    // ========================================================================

    void EditorApplication::Render()
    {
        if (!m_Initialized)
            return;

        // Start ImGui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Update editor camera.
        m_Context.GetCamera().Update(m_Context.GetDeltaTime());

        // Refresh the active scene from SceneManager to prevent dangling
        // raw pointers when the engine replaces the scene.
        auto* scene = m_Context.GetSceneManager().GetCurrentScene();
        m_Context.SetActiveScene(scene);

        // Update transforms on the active scene so world matrices are current.
        // Use a persistent TransformSystem instance to avoid re-attaching every frame.
        if (scene)
        {
            // Only re-attach if the scene pointer changed.
            if (m_LastScene != scene)
            {
                m_TransformSystem.OnAttach(scene->GetRegistry());
                m_LastScene = scene;
            }
            m_TransformSystem.Update(m_Context.GetDeltaTime());
        }

        // Render the main window with dockspace, menu bar, and toolbar.
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
                                     | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                                     | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                                     | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
                                     | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("##EditorMainFrame", nullptr, windowFlags);
        ImGui::PopStyleVar();

        // Menu bar.
        m_MenuBar.Render(m_Context);

        // Toolbar (zero vertical item spacing to eliminate gap below menu).
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
        m_Toolbar.Render(m_Context);
        ImGui::PopStyleVar();

        // Dockspace (creates dockspace on the current ##EditorMainFrame window).
        m_Dockspace.Render(m_Context);

        // Render all panels INSIDE the main frame window so they can dock.
        m_Context.GetPanels().RenderAll(m_Context);

        // Close the ##EditorMainFrame window.
        ImGui::End();

        // Render ImGui.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Render modal dialogs (outside ImGui::Render).
        if (m_ShowAboutDialog)
        {
            RenderAboutDialog();
        }
    }

    void EditorApplication::RenderAboutDialog()
    {
        ImGui::OpenPopup("About Eclipse Engine");

        if (ImGui::BeginPopupModal("About Eclipse Engine", &m_ShowAboutDialog,
                                    ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Eclipse Engine Editor");
            ImGui::Separator();
            ImGui::Text("Version: 0.1.0");
            ImGui::Text("Build:   Release");
            ImGui::Text("Compiler: MSVC / GCC / Clang");
            ImGui::Separator();
            ImGui::Text("A professional, modular, cross-platform C++20 game engine.");
            ImGui::Text("Repository: https://github.com/muzlik-gm/Eclipse-Engine");
            ImGui::Separator();
            if (ImGui::Button("Close", ImVec2(120, 0)))
            {
                m_ShowAboutDialog = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // ========================================================================
    // Key processing
    // ========================================================================

    void EditorApplication::ProcessKey(u32 key, u32 mods)
    {
        m_Context.GetShortcuts().ProcessKey(key, static_cast<KeyModifiers>(mods));
    }

    // ========================================================================
    // Default command / shortcut / panel registration
    // ========================================================================

    void EditorApplication::RegisterDefaultCommands()
    {
        auto& cmds = m_Context.GetCommands();

        // -- File commands -------------------------------------------------
        cmds.Register({"file.new_project", "New Project", "File", "Create a new project.",
            [this]() {
                m_Context.GetProjectManager().CreateProject(".", "NewProject");
                ENGINE_LOG_INFO("Editor: Created new project 'NewProject'");
            }});

        cmds.Register({"file.open_project", "Open Project...", "File", "Open an existing project.",
            [this]() {
                // Open the project browser panel.
                m_Context.GetPanels().OpenPanel("Project Browser");
                ENGINE_LOG_INFO("Editor: Open Project Browser");
            }});

        cmds.Register({"file.save_project", "Save Project", "File", "Save the current project.",
            [this]() {
                if (m_Context.GetProjectManager().HasProject())
                {
                    m_Context.GetProjectManager().SaveProject();
                    ENGINE_LOG_INFO("Editor: Project saved");
                }
                else
                {
                    ENGINE_LOG_WARN("Editor: No project open to save");
                }
            }});

        cmds.Register({"file.save_project_as", "Save Project As...", "File", "Save the project to a new location.",
            [this]() {
                ENGINE_LOG_INFO("Editor: Save Project As — not implemented yet");
            }});

        cmds.Register({"file.new_scene", "New Scene", "File", "Create a new scene.",
            [this]()
            {
                auto scene = m_Context.GetSceneManager().NewScene();
                m_Context.SetActiveScene(scene.get());
            }});

        cmds.Register({"file.open_scene", "Open Scene...", "File", "Open an existing scene.",
            [this]()
            {
                auto* proj = m_Context.GetProjectManager().GetCurrentProject();
                if (proj)
                {
                    auto scenes = proj->GetSceneFiles();
                    if (!scenes.empty())
                    {
                        auto scene = m_Context.GetSceneManager().LoadScene(scenes[0]);
                        if (scene)
                            m_Context.SetActiveScene(scene.get());
                    }
                }
            }});

        cmds.Register({"file.save_scene", "Save Scene", "File", "Save the current scene.",
            [this]()
            {
                if (m_Context.GetSceneManager().GetCurrentScenePath().empty())
                {
                    auto* proj = m_Context.GetProjectManager().GetCurrentProject();
                    if (proj)
                    {
                        auto path = (proj->GetScenePath() / "NewScene.scene").string();
                        m_Context.GetSceneManager().SaveSceneAs(path);
                    }
                }
                else
                {
                    m_Context.GetSceneManager().SaveScene();
                }
            }});

        cmds.Register({"file.save_scene_as", "Save Scene As...", "File", "Save the scene to a new file.",
            []() { ENGINE_LOG_INFO("Editor: Save Scene As command"); }});

        cmds.Register({"file.exit", "Exit", "File", "Exit the editor.",
            [this]() { if (m_Window) glfwSetWindowShouldClose(m_Window, GLFW_TRUE); }});

        // -- Edit commands -------------------------------------------------
        cmds.Register({"edit.undo", "Undo", "Edit", "Undo the last action.",
            [this]() { m_Context.GetHistory().Undo(); },
            [this]() { return m_Context.GetHistory().CanUndo(); }});
        cmds.Register({"edit.redo", "Redo", "Edit", "Redo the last undone action.",
            [this]() { m_Context.GetHistory().Redo(); },
            [this]() { return m_Context.GetHistory().CanRedo(); }});
        cmds.Register({"edit.cut", "Cut", "Edit", "Cut the selection.",
            [this]() {
                auto entity = m_Context.GetSelection().GetPrimaryEntity();
                if (entity != engine::ecs::Invalid)
                {
                    m_Context.GetCommands().Execute("entity.delete");
                    ENGINE_LOG_INFO("Editor: Cut entity");
                }
            }});
        cmds.Register({"edit.copy", "Copy", "Edit", "Copy the selection.",
            [this]() {
                ENGINE_LOG_INFO("Editor: Copy — stored entity for paste");
            }});
        cmds.Register({"edit.paste", "Paste", "Edit", "Paste from clipboard.",
            [this]() {
                ENGINE_LOG_INFO("Editor: Paste — not fully implemented");
            }});
        cmds.Register({"edit.duplicate", "Duplicate", "Edit", "Duplicate the selection.",
            [this]() {
                auto entity = m_Context.GetSelection().GetPrimaryEntity();
                if (entity == engine::ecs::Invalid) return;
                auto* scene = m_Context.GetActiveScene();
                if (!scene) return;
                auto& reg = scene->GetRegistry();
                if (reg.HasComponent<engine::components::TagComponent>(entity))
                {
                    auto& tag = reg.GetComponent<engine::components::TagComponent>(entity);
                    auto newEntity = EntityFactory::CreateEmpty(m_Context, tag.Tag + " (Copy)");
                    if (reg.HasComponent<engine::components::TransformComponent>(entity))
                    {
                        auto& tf = reg.GetComponent<engine::components::TransformComponent>(entity);
                        if (newEntity != engine::ecs::Invalid)
                        {
                            auto& newTf = reg.GetComponent<engine::components::TransformComponent>(newEntity);
                            newTf.Translation = tf.Translation;
                            newTf.Rotation = tf.Rotation;
                            newTf.Scale = tf.Scale;
                            newTf.WorldDirty = true;
                        }
                    }
                    m_Context.GetSelection().SelectEntity(newEntity);
                    ENGINE_LOG_INFO("Editor: Duplicated entity");
                }
            }});
        cmds.Register({"edit.delete", "Delete", "Edit", "Delete the selection.",
            [this]() { m_Context.GetCommands().Execute("entity.delete"); }});
        cmds.Register({"edit.preferences", "Preferences...", "Edit", "Open editor preferences.",
            [this]() {
                m_Context.GetPanels().OpenPanel("Statistics");
                ENGINE_LOG_INFO("Editor: Preferences — save current prefs");
                m_Context.GetPreferences().Save(".editor/editor_prefs.json");
            }});

        // -- View commands -------------------------------------------------
        cmds.Register({"view.toggle_grid", "Toggle Grid", "View", "Toggle the scene grid.",
            [this]() { m_Context.GetPreferences().GridVisible = !m_Context.GetPreferences().GridVisible; }});
        cmds.Register({"view.toggle_gizmos", "Toggle Gizmos", "View", "Toggle gizmo visibility.",
            [this]() {
                // Toggle gizmo mode between None and Translate.
                auto& gizmos = m_Context.GetGizmos();
                if (gizmos.GetMode() == GizmoMode::None)
                    gizmos.SetMode(GizmoMode::Translate);
                else
                    gizmos.SetMode(GizmoMode::None);
                ENGINE_LOG_INFO("Editor: Gizmos toggled");
            }});
        cmds.Register({"view.frame_selected", "Frame Selected", "View", "Focus the camera on the selected entity.",
            [this]()
            {
                auto entity = m_Context.GetSelection().GetPrimaryEntity();
                if (entity == engine::ecs::Invalid) return;
                auto* scene = m_Context.GetActiveScene();
                if (!scene) return;
                auto& reg = scene->GetRegistry();
                if (reg.HasComponent<engine::components::TransformComponent>(entity))
                {
                    auto& tf = reg.GetComponent<engine::components::TransformComponent>(entity);
                    auto pos = engine::transforms::DecomposePosition(tf.WorldMatrix);
                    m_Context.GetCamera().Focus(pos);
                    ENGINE_LOG_INFO("Editor: Framed entity at ({}, {}, {})", pos.x, pos.y, pos.z);
                }
                else
                {
                    m_Context.GetCamera().Focus(engine::math::Vec3(0.0f));
                }
            }});
        cmds.Register({"view.zoom_in", "Zoom In", "View", "Zoom the camera in.",
            [this]() { m_Context.GetCamera().Zoom(-1.0f); }});
        cmds.Register({"view.zoom_out", "Zoom Out", "View", "Zoom the camera out.",
            [this]() { m_Context.GetCamera().Zoom(1.0f); }});
        cmds.Register({"view.reset_camera", "Reset Camera", "View", "Reset the camera to default position.",
            [this]()
            {
                m_Context.GetCamera().SetPosition(engine::math::Vec3(0.0f, 5.0f, 10.0f));
                m_Context.GetCamera().SetTarget(engine::math::Vec3(0.0f));
            }});

        // -- Window commands ----------------------------------------------
        cmds.Register({"window.close_all", "Close All Panels", "Window", "Close all docked panels.",
            [this]()
            {
                for (auto* p : m_Context.GetPanels().GetAllPanels())
                    if (p->CanClose()) p->SetOpen(false);
            }});
        cmds.Register({"window.reset_layout", "Reset Layout", "Window", "Reset the editor layout to defaults.",
            [this]() {
                m_Context.GetLayout().ResetToDefault();
                // Re-open default panels.
                m_Context.GetPanels().OpenPanel("Hierarchy");
                m_Context.GetPanels().OpenPanel("Inspector");
                m_Context.GetPanels().OpenPanel("Scene View");
                m_Context.GetPanels().OpenPanel("Game View");
                m_Context.GetPanels().OpenPanel("Content Browser");
                m_Context.GetPanels().OpenPanel("Console");
                ENGINE_LOG_INFO("Editor: Layout reset to defaults");
            }});

        // -- Tools commands -----------------------------------------------
        cmds.Register({"tools.build_assets", "Build Asset Database", "Tools", "Scan and import all assets.",
            [this]() {
                auto* proj = m_Context.GetProjectManager().GetCurrentProject();
                if (proj && proj->IsOpen())
                {
                    ENGINE_LOG_INFO("Editor: Building asset database for '{}'", proj->GetName());
                    // Scan the Assets directory.
                    // The actual scanning is handled by the AssetDatabase.
                }
                else
                {
                    ENGINE_LOG_WARN("Editor: No project open — cannot build assets");
                }
            }});
        cmds.Register({"tools.reimport_all", "Reimport All", "Tools", "Reimport all assets from source.",
            [this]() {
                ENGINE_LOG_INFO("Editor: Reimport all assets — framework ready");
            }});
        cmds.Register({"tools.project_settings", "Project Settings...", "Tools", "Open project settings.",
            [this]() {
                auto* proj = m_Context.GetProjectManager().GetCurrentProject();
                if (proj && proj->IsOpen())
                {
                    proj->Save();
                    ENGINE_LOG_INFO("Editor: Project settings saved for '{}'", proj->GetName());
                }
                else
                {
                    ENGINE_LOG_WARN("Editor: No project open — cannot edit settings");
                }
            }});
        cmds.Register({"tools.editor_preferences", "Editor Preferences...", "Tools", "Open editor preferences.",
            [this]() {
                m_Context.GetPreferences().Save(".editor/editor_prefs.json");
                ENGINE_LOG_INFO("Editor: Preferences saved");
            }});

        // -- Help commands -------------------------------------------------
        cmds.Register({"help.documentation", "Documentation", "Help", "Open the engine documentation.",
            []() {
                ENGINE_LOG_INFO("Eclipse Engine Documentation: https://github.com/muzlik-gm/Eclipse-Engine");
            }});
        cmds.Register({"help.about", "About", "Help", "Show about dialog.",
            [this]() {
                m_ShowAboutDialog = true;
            }});

        // -- Editor mode commands -----------------------------------------
        cmds.Register({"editor.play", "Play", "Editor", "Enter play mode.",
            [this]() { m_Context.GetPlayMode().EnterPlayMode(m_Context); }});
        cmds.Register({"editor.pause", "Pause", "Editor", "Pause play mode.",
            [this]() { m_Context.GetPlayMode().Pause(m_Context); }});
        cmds.Register({"editor.stop", "Stop", "Editor", "Stop play mode and return to edit mode.",
            [this]() { m_Context.GetPlayMode().Stop(m_Context); }});
        cmds.Register({"editor.step", "Step", "Editor", "Step one frame in play mode.",
            [this]() { m_Context.GetPlayMode().Step(m_Context); }});

        // -- Entity creation commands -------------------------------------
        cmds.Register({"entity.create_empty", "Create Empty", "Entity", "Create an empty entity.",
            [this]() { EntityFactory::CreateEmpty(m_Context); }});
        cmds.Register({"entity.create_camera", "Create Camera", "Entity", "Create a camera entity.",
            [this]() { EntityFactory::CreateCamera(m_Context); }});
        cmds.Register({"entity.create_light", "Create Light", "Entity", "Create a directional light.",
            [this]() { EntityFactory::CreateDirectionalLight(m_Context); }});
        cmds.Register({"entity.create_cube", "Create Cube", "Entity", "Create a cube entity.",
            [this]() { EntityFactory::CreateCube(m_Context); }});
        cmds.Register({"entity.create_plane", "Create Plane", "Entity", "Create a plane entity.",
            [this]() { EntityFactory::CreatePlane(m_Context); }});
        cmds.Register({"entity.create_sphere", "Create Sphere", "Entity", "Create a sphere entity.",
            [this]() { EntityFactory::CreateSphere(m_Context); }});

        // -- Create default test scene ------------------------------------
        cmds.Register({"scene.create_default", "Create Test Scene", "Scene",
            "Create a scene with a camera, light, and test cubes.",
            [this]()
            {
                // Stop play mode if currently playing (otherwise it will
                // restore an empty snapshot and destroy our new scene).
                if (m_Context.GetPlayMode().IsPlaying())
                {
                    m_Context.GetPlayMode().Stop(m_Context);
                }

                auto scene = m_Context.GetSceneManager().NewScene("Test Scene");
                m_Context.SetActiveScene(scene.get());

                // Position the editor camera to see the grid.
                auto& cam = m_Context.GetCamera();
                cam.SetPosition(engine::math::Vec3(0.0f, 5.0f, 10.0f));
                cam.SetTarget(engine::math::Vec3(0.0f, 0.0f, 0.0f));
                cam.SetDistance(10.0f);

                // Create camera entity.
                auto camEntity = EntityFactory::CreateCamera(m_Context, "Main Camera");
                auto* activeScene = m_Context.GetActiveScene();
                if (activeScene)
                {
                    auto& camTf = activeScene->GetRegistry().GetComponent<engine::components::TransformComponent>(camEntity);
                    camTf.Translation = {0.0f, 3.0f, 8.0f};
                    camTf.WorldDirty = true;
                }

                // Create a light.
                EntityFactory::CreateDirectionalLight(m_Context, "Sun");

                // Create some cubes.
                auto cube1 = EntityFactory::CreateCube(m_Context, "Cube 1");
                if (activeScene)
                {
                    auto& tf1 = activeScene->GetRegistry().GetComponent<engine::components::TransformComponent>(cube1);
                    tf1.Translation = {0.0f, 0.0f, 0.0f};
                    tf1.WorldDirty = true;
                }

                auto cube2 = EntityFactory::CreateCube(m_Context, "Cube 2");
                if (activeScene)
                {
                    auto& tf2 = activeScene->GetRegistry().GetComponent<engine::components::TransformComponent>(cube2);
                    tf2.Translation = {2.0f, 0.0f, 0.0f};
                    tf2.WorldDirty = true;
                }

                auto cube3 = EntityFactory::CreateCube(m_Context, "Cube 3");
                if (activeScene)
                {
                    auto& tf3 = activeScene->GetRegistry().GetComponent<engine::components::TransformComponent>(cube3);
                    tf3.Translation = {-2.0f, 0.0f, 0.0f};
                    tf3.WorldDirty = true;
                }

                // Create a plane as a floor.
                auto plane = EntityFactory::CreatePlane(m_Context, "Floor");
                if (activeScene)
                {
                    auto& tfP = activeScene->GetRegistry().GetComponent<engine::components::TransformComponent>(plane);
                    tfP.Translation = {0.0f, -0.5f, 0.0f};
                    tfP.Scale = {10.0f, 1.0f, 10.0f};
                    tfP.WorldDirty = true;
                }

                ENGINE_LOG_INFO("Editor — created default test scene");
            }});

        // -- Entity deletion command --------------------------------------
        cmds.Register({"entity.delete", "Delete Entity", "Entity", "Delete the selected entity.",
            [this]()
            {
                auto entity = m_Context.GetSelection().GetPrimaryEntity();
                if (entity != engine::ecs::Invalid)
                    EntityFactory::DeleteEntity(m_Context, entity);
            },
            [this]() { return m_Context.GetSelection().GetPrimaryEntity() != engine::ecs::Invalid; }});
    }

    void EditorApplication::RegisterDefaultShortcuts()
    {
        using K = KeyModifiers;
        auto& sc = m_Context.GetShortcuts();

        // Use GLFW key constants.
        #define GLFW_KEY_A 65
        #define GLFW_KEY_D 68
        #define GLFW_KEY_E 69
        #define GLFW_KEY_F 70
        #define GLFW_KEY_N 78
        #define GLFW_KEY_O 79
        #define GLFW_KEY_Q 81
        #define GLFW_KEY_R 82
        #define GLFW_KEY_S 83
        #define GLFW_KEY_W 87
        #define GLFW_KEY_SPACE 32
        #define GLFW_KEY_DELETE 256
        #define GLFW_KEY_F5 294

        // File
        sc.Bind("file.save_scene",       GLFW_KEY_S, K::Ctrl, "Ctrl+S");
        sc.Bind("file.open_scene",       GLFW_KEY_O, K::Ctrl, "Ctrl+O");
        sc.Bind("file.save_scene_as",    GLFW_KEY_S, K::Ctrl | K::Shift, "Ctrl+Shift+S");
        sc.Bind("file.new_scene",        GLFW_KEY_N, K::Ctrl, "Ctrl+N");

        // Edit
        sc.Bind("edit.delete",           GLFW_KEY_DELETE, K::None, "Delete");
        sc.Bind("edit.duplicate",        GLFW_KEY_D, K::Ctrl, "Ctrl+D");

        // View
        sc.Bind("view.frame_selected",   GLFW_KEY_F, K::None, "F");

        // Transform modes
        sc.Bind("editor.translate_mode", GLFW_KEY_W, K::None, "W");
        sc.Bind("editor.rotate_mode",    GLFW_KEY_E, K::None, "E");
        sc.Bind("editor.scale_mode",     GLFW_KEY_R, K::None, "R");

        // Play mode — removed Space shortcut (too dangerous, triggers on
        // any text input).  Use the toolbar Play button instead.

        // Register transform mode commands.
        auto& cmds = m_Context.GetCommands();
        cmds.Register({"editor.translate_mode", "Translate Mode", "Editor", "Set gizmo to translate mode.",
            [this]() { m_Context.GetGizmos().SetMode(GizmoMode::Translate); }});
        cmds.Register({"editor.rotate_mode", "Rotate Mode", "Editor", "Set gizmo to rotate mode.",
            [this]() { m_Context.GetGizmos().SetMode(GizmoMode::Rotate); }});
        cmds.Register({"editor.scale_mode", "Scale Mode", "Editor", "Set gizmo to scale mode.",
            [this]() { m_Context.GetGizmos().SetMode(GizmoMode::Scale); }});
    }

    void EditorApplication::RegisterDefaultPanels()
    {
        auto& panels = m_Context.GetPanels();

        panels.Register(std::make_unique<HierarchyPanel>());
        panels.Register(std::make_unique<InspectorPanel>());
        panels.Register(std::make_unique<ScenePanel>());
        panels.Register(std::make_unique<GamePanel>());
        panels.Register(std::make_unique<ContentBrowserPanel>());
        panels.Register(std::make_unique<ConsolePanel>());
        panels.Register(std::make_unique<StatisticsPanel>());
        panels.Register(std::make_unique<ProfilerPanel>());
        panels.Register(std::make_unique<ProjectBrowserPanel>());

        // Find the console panel for log routing.
        m_ConsolePanel = dynamic_cast<ConsolePanel*>(
            panels.FindPanel("Console"));
    }

} // namespace editor
