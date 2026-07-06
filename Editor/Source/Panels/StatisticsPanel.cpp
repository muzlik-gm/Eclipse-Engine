// ============================================================================
// File: Editor/Source/Panels/StatisticsPanel.cpp
// ============================================================================
#include "Editor/Panels/StatisticsPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Renderer/Core/Renderer.h"
#include "Engine/Renderer/Core/RendererContext.h"

#include <imgui.h>

namespace editor {

    using engine::core::u32;
    using engine::core::f32;
    using engine::core::f64;

    StatisticsPanel::StatisticsPanel() = default;

    void StatisticsPanel::OnRender(EditorContext& context)
    {
        // -- Frame stats ---------------------------------------------------
        if (ImGui::CollapsingHeader("Frame", ImGuiTreeNodeFlags_DefaultOpen))
        {
            f64 dt = context.GetDeltaTime();
            f32 fps = dt > 0.0 ? static_cast<f32>(1.0 / dt) : 0.0f;

            ImGui::Text("FPS:        %.1f", fps);
            ImGui::Text("Frame Time: %.2f ms", dt * 1000.0);
            ImGui::Text("CPU Time:   %.2f ms (placeholder)", dt * 1000.0);
            ImGui::Text("GPU Time:   %.2f ms (placeholder)", 0.0);
        }

        // -- Rendering stats ----------------------------------------------
        if (ImGui::CollapsingHeader("Rendering"))
        {
            auto* renderer = context.GetRenderer();
            if (renderer)
            {
                const auto& stats = renderer->GetContext().GetStatistics();
                ImGui::Text("Draw Calls:       %u", stats.DrawCalls + stats.DrawIndexedCalls);
                ImGui::Text("Triangles:        %u", stats.TrianglesDrawn);
                ImGui::Text("Vertices:         %u", stats.VerticesDrawn);
                ImGui::Text("Bind Pipeline:    %u", stats.BindPipelineCalls);
                ImGui::Text("Bind Texture:     %u", stats.BindTextureCalls);
                ImGui::Text("Bind Framebuffer: %u", stats.BindFramebufferCalls);
            }
            else
            {
                ImGui::TextDisabled("Renderer not available");
            }
        }

        // -- Entity stats --------------------------------------------------
        if (ImGui::CollapsingHeader("Entities"))
        {
            auto* scene = context.GetActiveScene();
            if (scene)
            {
                u32 entityCount = 0;
                auto& reg = scene->GetRegistry();
                for ([[maybe_unused]] auto e : reg.View<engine::components::TagComponent>())
                    ++entityCount;

                ImGui::Text("Entity Count: %u", entityCount);
            }
            else
            {
                ImGui::TextDisabled("No active scene");
            }
        }

        // -- Memory stats --------------------------------------------------
        if (ImGui::CollapsingHeader("Memory"))
        {
            ImGui::Text("Texture Uploads:  %u bytes (placeholder)", 0u);
            ImGui::Text("Buffer Uploads:   %u bytes (placeholder)", 0u);
            ImGui::Text("Total Allocated:  %u bytes (placeholder)", 0u);
        }
    }

} // namespace editor
