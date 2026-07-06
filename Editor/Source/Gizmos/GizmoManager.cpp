// ============================================================================
// File: Editor/Source/Gizmos/GizmoManager.cpp
// ============================================================================
#include "Editor/Gizmos/GizmoManager.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Selection/EditorSelection.h"

namespace editor {

    GizmoManager::GizmoManager() = default;

    void GizmoManager::Render(EditorContext& context)
    {
        // The gizmo renders a manipulator for the selected entity.
        // Actual manipulator rendering uses ImGuizmo or a custom draw.
        // This framework is the architecture — the rendering backend
        // is plugged in during the rendering systems phase.

        auto entity = context.GetSelection().GetPrimaryEntity();
        if (entity == engine::ecs::Invalid)
            return;

        if (m_Mode == GizmoMode::None)
            return;

        // The actual gizmo drawing will be implemented when the
        // rendering systems phase provides line/primitive drawing
        // through the RHI.  For now, the framework is in place.
    }

} // namespace editor
