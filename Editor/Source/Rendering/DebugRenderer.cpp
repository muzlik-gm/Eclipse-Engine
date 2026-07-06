// ============================================================================
// File: Editor/Source/Rendering/DebugRenderer.cpp
// ============================================================================
#include "Editor/Rendering/DebugRenderer.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Selection/EditorSelection.h"
#include "Editor/Theme/ThemeManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/HierarchyComponent.h"

#include <glad/glad.h>
#include <imgui.h>

namespace editor {

    using engine::core::f32;
    using engine::math::Vec3;
    using engine::math::Vec4;
    using engine::math::Mat4;

    // ========================================================================
    // Helper: draw a line in screen space via ImGui (simplified approach).
    // In production, this would use a proper line shader with the
    // view-projection matrix.
    // ========================================================================

    void DebugRenderer::RenderGrid(const Vec3& center, float size,
                                    int divisions, const Vec4& color)
    {
        // For now, render the grid using ImGui's draw list since we
        // don't have a line shader wired up yet.  This is a screen-space
        // approximation.
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + size, p0.y + size);

        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w));

        float step = size / static_cast<float>(divisions);
        for (int i = 0; i <= divisions; ++i)
        {
            float x = p0.x + step * i;
            dl->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), col, 1.0f);
            float y = p0.y + step * i;
            dl->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), col, 1.0f);
        }

        (void)center;
    }

    void DebugRenderer::RenderOriginAxes(float axisLength)
    {
        // Placeholder — renders axis labels via ImGui.
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center = ImGui::GetCursorScreenPos();
        center.x += 50;
        center.y += 50;

        // X axis (red)
        dl->AddLine(center, ImVec2(center.x + axisLength * 20, center.y),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.2f, 0.2f, 1.0f)), 2.0f);
        dl->AddText(ImVec2(center.x + axisLength * 20 + 4, center.y - 8),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.2f, 0.2f, 1.0f)), "X");

        // Y axis (green)
        dl->AddLine(center, ImVec2(center.x, center.y - axisLength * 20),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.9f, 0.2f, 1.0f)), 2.0f);
        dl->AddText(ImVec2(center.x + 4, center.y - axisLength * 20 - 16),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.9f, 0.2f, 1.0f)), "Y");
    }

    void DebugRenderer::RenderBoundingBox(const Vec3& min, const Vec3& max,
                                           const Vec4& color)
    {
        (void)min; (void)max; (void)color;
        // Full 3D bounding box rendering requires a line shader.
        // Framework is in place — the rendering systems phase will
        // provide the line drawing pipeline.
    }

    void DebugRenderer::RenderSelectionOutline(EditorContext& context, engine::ecs::Entity entity)
    {
        if (entity == engine::ecs::Invalid)
            return;

        auto* scene = context.GetActiveScene();
        if (!scene)
            return;

        auto& registry = scene->GetRegistry();
        if (!registry.HasComponent<engine::components::TransformComponent>(entity))
            return;

        auto& tf = registry.GetComponent<engine::components::TransformComponent>(entity);

        // Render a simple bounding box around the entity.
        // The framework is in place — full 3D rendering is wired later.
        (void)tf;
    }

    void DebugRenderer::RenderCameraFrustum(const Mat4& viewProjection, const Vec4& color)
    {
        (void)viewProjection; (void)color;
        // Framework in place.
    }

    void DebugRenderer::RenderLine(const Vec3& p0, const Vec3& p1, const Vec4& color)
    {
        (void)p0; (void)p1; (void)color;
        // Framework in place.
    }

} // namespace editor
