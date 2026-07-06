// ============================================================================
// File: Editor/Include/Editor/Core/PlayModeController.h
// Manages Play/Pause/Stop/Step mode with editor↔runtime world separation.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Serialization/SceneSerializer.h"

#include <memory>
#include <string>

namespace editor {

    class EditorContext;

    /// @brief Manages the editor play mode lifecycle.  When entering play
    ///        mode, a runtime copy of the editor scene is created so that
    ///        gameplay simulation does not modify the editor world.
    ///        Stopping play mode restores the editor scene.
    class PlayModeController
    {
    public:
        PlayModeController();
        ~PlayModeController() = default;

        /// @brief Enters play mode.  Serializes the editor scene and
        ///        creates a runtime copy.
        void EnterPlayMode(EditorContext& context);

        /// @brief Pauses the simulation (entities freeze, no updates).
        void Pause(EditorContext& context);

        /// @brief Resumes the simulation from a paused state.
        void Resume(EditorContext& context);

        /// @brief Stops play mode.  Restores the editor scene from the
        ///        saved snapshot.
        void Stop(EditorContext& context);

        /// @brief Steps a single frame while paused.
        void Step(EditorContext& context);

        /// @brief Returns true if currently in play mode.
        [[nodiscard]] bool IsPlaying() const noexcept { return m_Playing; }

        /// @brief Returns true if currently paused.
        [[nodiscard]] bool IsPaused() const noexcept { return m_Paused; }

        /// @brief Returns true if a single-frame step is pending.
        [[nodiscard]] bool IsStepping() const noexcept { return m_Stepping; }

        /// @brief Returns the runtime simulation time (seconds since play started).
        [[nodiscard]] engine::core::f64 GetSimulationTime() const noexcept
        { return m_SimulationTime; }

    private:
        bool                  m_Playing{false};
        bool                  m_Paused{false};
        bool                  m_Stepping{false};
        engine::core::f64     m_SimulationTime{0.0};

        // Serialized editor scene for restore on stop.
        std::string           m_EditorSceneSnapshot;
    };

} // namespace editor
