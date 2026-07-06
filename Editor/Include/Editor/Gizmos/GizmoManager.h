// ============================================================================
// File: Editor/Include/Editor/Gizmos/GizmoManager.h
// Gizmo framework — translate/rotate/scale manipulators.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/ECS/Entity.h"

namespace editor {

    class EditorContext;

    // ========================================================================
    // GizmoMode — which manipulator is active.
    // ========================================================================

    enum class GizmoMode : engine::core::u32
    {
        None     = 0,
        Translate = 1,
        Rotate   = 2,
        Scale    = 3
    };

    // ========================================================================
    // GizmoSpace — local or world space.
    // ========================================================================

    enum class GizmoSpace : engine::core::u32
    {
        Local = 0,
        World = 1
    };

    // ========================================================================
    // GizmoManager — manages gizmo rendering and interaction.
    // ========================================================================

    /// @brief Framework for gizmo manipulators (translate/rotate/scale).
    ///        Actual rendering uses ImGui's ImGuizmo or a custom
    ///        implementation.  This framework provides the architecture
    ///        for mode switching, snapping, and space selection.
    class GizmoManager
    {
    public:
        GizmoManager();
        ~GizmoManager() = default;

        /// @brief Renders the gizmo for the currently selected entity.
        void Render(EditorContext& context);

        // -- Mode ----------------------------------------------------------

        [[nodiscard]] GizmoMode GetMode() const noexcept { return m_Mode; }
        void SetMode(GizmoMode mode) noexcept { m_Mode = mode; }

        [[nodiscard]] GizmoSpace GetSpace() const noexcept { return m_Space; }
        void SetSpace(GizmoSpace space) noexcept { m_Space = space; }

        // -- Snapping ------------------------------------------------------

        [[nodiscard]] bool IsSnappingEnabled() const noexcept { return m_Snapping; }
        void SetSnapping(bool enabled) noexcept { m_Snapping = enabled; }

        [[nodiscard]] float GetTranslateSnap() const noexcept { return m_TranslateSnap; }
        void SetTranslateSnap(float v) noexcept { m_TranslateSnap = v; }

        [[nodiscard]] float GetRotateSnap() const noexcept { return m_RotateSnap; }
        void SetRotateSnap(float v) noexcept { m_RotateSnap = v; }

        [[nodiscard]] float GetScaleSnap() const noexcept { return m_ScaleSnap; }
        void SetScaleSnap(float v) noexcept { m_ScaleSnap = v; }

        // -- Axis colors ---------------------------------------------------

        [[nodiscard]] static engine::math::Vec4 GetAxisXColor() noexcept
        { return {0.90f, 0.25f, 0.25f, 1.0f}; }

        [[nodiscard]] static engine::math::Vec4 GetAxisYColor() noexcept
        { return {0.25f, 0.90f, 0.35f, 1.0f}; }

        [[nodiscard]] static engine::math::Vec4 GetAxisZColor() noexcept
        { return {0.25f, 0.50f, 0.95f, 1.0f}; }

    private:
        GizmoMode m_Mode{GizmoMode::Translate};
        GizmoSpace m_Space{GizmoSpace::World};
        bool      m_Snapping{false};
        float     m_TranslateSnap{0.5f};
        float     m_RotateSnap{15.0f};
        float     m_ScaleSnap{0.1f};
    };

} // namespace editor
