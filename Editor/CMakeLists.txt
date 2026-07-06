# Editor target — Eclipse Engine Editor Application.
set(EDITOR_SOURCES
    Source/EditorMain.cpp
    Source/Core/EditorContext.cpp
    Source/Core/EditorApplication.cpp
    Source/Core/Dockspace.cpp
    Source/Core/MenuBar.cpp
    Source/Core/Toolbar.cpp
    Source/Core/EditorIcons.cpp
    Source/Core/EntityFactory.cpp
    Source/Core/PlayModeController.cpp
    Source/Commands/EditorCommandSystem.cpp
    Source/Commands/ShortcutManager.cpp
    Source/Theme/ThemeManager.cpp
    Source/Framework/PanelManager.cpp
    Source/Framework/LayoutManager.cpp
    Source/Selection/EditorSelection.cpp
    Source/Camera/EditorCamera.cpp
    Source/Gizmos/GizmoManager.cpp
    Source/Prefs/EditorPreferences.cpp
    Source/Prefs/ProjectSettings.cpp
    Source/Panels/HierarchyPanel.cpp
    Source/Panels/InspectorPanel.cpp
    Source/Panels/ScenePanel.cpp
    Source/Panels/GamePanel.cpp
    Source/Panels/ConsolePanel.cpp
    Source/Panels/ContentBrowserPanel.cpp
    Source/Panels/StatisticsPanel.cpp
    Source/Panels/ProfilerPanel.cpp
    Source/Panels/ProjectBrowserPanel.cpp
    Source/Viewport/ViewportFramebuffer.cpp
    Source/Viewport/FramebufferManager.cpp
    Source/Rendering/SceneRenderer.cpp
    Source/Rendering/DebugRenderer.cpp
    Source/Picking/EntityPicking.cpp
    Source/Project/Project.cpp
    Source/Project/ProjectManager.cpp
    Source/Project/SceneManager.cpp
    Source/History/CommandHistory.cpp
    Source/Core/DragDrop.cpp
)

add_executable(EclipseEditor ${EDITOR_SOURCES})

target_include_directories(EclipseEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_link_libraries(EclipseEditor
    PRIVATE
        Engine
        imgui
        glfw
        glad_api
)

set_target_properties(EclipseEditor PROPERTIES
    FOLDER "Editor"
    OUTPUT_NAME "EclipseEditor"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(ENGINE_PLATFORM_LINUX)
    target_link_libraries(EclipseEditor PRIVATE dl)
endif()
