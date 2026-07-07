// ============================================================================
// File: Editor/Include/Editor/Core/EditorApplication.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Application/Application.h"
#include "Engine/Systems/TransformSystem.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Core/Dockspace.h"
#include "Editor/Core/MenuBar.h"
#include "Editor/Core/Toolbar.h"

#include <memory>

struct GLFWwindow;

namespace editor {

    /// @brief The Editor application.  Owns the EditorContext, dockspace,
    ///        menu bar, toolbar, and all panels.  Separates editor logic
    ///        from runtime simulation (which stays inside the Engine).
    class EditorApplication
    {
    public:
        EditorApplication();
        ~EditorApplication();

        EditorApplication(const EditorApplication&)            = delete;
        EditorApplication& operator=(const EditorApplication&) = delete;
        EditorApplication(EditorApplication&&)                 = delete;
        EditorApplication& operator=(EditorApplication&&)      = delete;

        /// @brief Initializes the editor.
        /// @param window The GLFW window to render into.
        /// @param eventBus The engine event bus.
        /// @param scene The active scene.
        /// @param renderer The renderer.
        bool Initialize(GLFWwindow* window, engine::events::EventBus* eventBus,
                         engine::scene::Scene* scene, engine::renderer::Renderer* renderer);

        /// @brief Shuts down the editor.
        void Shutdown();

        /// @brief Renders one editor frame.  Call every frame after
        ///        the runtime update.
        void Render();

        /// @brief Processes a key event for shortcuts.
        void ProcessKey(engine::core::u32 key, engine::core::u32 mods);

        /// @brief Returns the editor context.
        [[nodiscard]] EditorContext& GetContext() noexcept { return m_Context; }

    private:
        void RegisterDefaultCommands();
        void RegisterDefaultShortcuts();
        void RegisterDefaultPanels();
        void InitializeImGui(GLFWwindow* window);
        void ShutdownImGui();

        EditorContext  m_Context;
        Dockspace      m_Dockspace;
        MenuBar        m_MenuBar;
        Toolbar        m_Toolbar;

        GLFWwindow*    m_Window{nullptr};
        bool           m_Initialized{false};

        // Persistent transform system (avoids re-attaching every frame).
        engine::systems::TransformSystem m_TransformSystem;
        engine::scene::Scene*           m_LastScene{nullptr};

        // Console panel reference (for log routing).
        class ConsolePanel* m_ConsolePanel{nullptr};

        // About dialog state.
        bool m_ShowAboutDialog{false};

        // Renders the About dialog.
        void RenderAboutDialog();
    };

} // namespace editor
