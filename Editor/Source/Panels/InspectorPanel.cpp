// ============================================================================
// File: Editor/Source/Panels/InspectorPanel.cpp
// ============================================================================
#include "Editor/Panels/InspectorPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Selection/EditorSelection.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/NameComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Components/EnabledComponent.h"
#include "Engine/Components/StaticComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/LightComponent.h"

#include <imgui.h>

namespace editor {

    using engine::core::u32;

    InspectorPanel::InspectorPanel() = default;

    void InspectorPanel::OnRender(EditorContext& context)
    {
        auto entity = context.GetSelection().GetPrimaryEntity();
        if (entity == engine::ecs::Invalid)
        {
            ImGui::TextDisabled("No entity selected");
            return;
        }

        auto* scene = context.GetActiveScene();
        if (!scene)
        {
            ImGui::TextDisabled("No active scene");
            return;
        }

        RenderEntityInfo(context, entity, *scene);
        ImGui::Separator();
        RenderTransform(context, entity, *scene);
        ImGui::Separator();
        RenderComponents(context, entity, *scene);
    }

    void InspectorPanel::RenderEntityInfo(EditorContext& context, engine::ecs::Entity entity,
                                           engine::scene::Scene& scene)
    {
        auto& registry = scene.GetRegistry();

        // Entity name (TagComponent)
        if (registry.HasComponent<engine::components::TagComponent>(entity))
        {
            auto& tag = registry.GetComponent<engine::components::TagComponent>(entity);
            char buf[256] = {};
            std::copy(tag.Tag.begin(), tag.Tag.end(), buf);
            if (ImGui::InputText("Name", buf, sizeof(buf)))
                tag.Tag = buf;
        }

        // Entity UUID
        if (registry.HasComponent<engine::components::IDComponent>(entity))
        {
            auto& id = registry.GetComponent<engine::components::IDComponent>(entity);
            ImGui::Text("UUID: %s", id.ID.ToString().c_str());
        }

        // Entity ID
        ImGui::Text("Entity ID: %u", static_cast<u32>(entity));
    }

    void InspectorPanel::RenderTransform(EditorContext& context, engine::ecs::Entity entity,
                                          engine::scene::Scene& scene)
    {
        auto& registry = scene.GetRegistry();

        if (!registry.HasComponent<engine::components::TransformComponent>(entity))
            return;

        auto& tf = registry.GetComponent<engine::components::TransformComponent>(entity);

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Position", &tf.Translation.x, 0.1f);

            auto euler = tf.GetEulerDegrees();
            if (ImGui::DragFloat3("Rotation", &euler.x, 0.5f))
                tf.SetEulerDegrees(euler);

            ImGui::DragFloat3("Scale", &tf.Scale.x, 0.1f);
            tf.WorldDirty = true;
        }
    }

    void InspectorPanel::RenderComponents(EditorContext& context, engine::ecs::Entity entity,
                                           engine::scene::Scene& scene)
    {
        auto& registry = scene.GetRegistry();

        // Visibility
        if (registry.HasComponent<engine::components::VisibilityComponent>(entity))
        {
            auto& vis = registry.GetComponent<engine::components::VisibilityComponent>(entity);
            if (ImGui::CollapsingHeader("Visibility"))
            {
                ImGui::Checkbox("Visible", &vis.IsVisible);
            }
        }

        // Enabled
        if (registry.HasComponent<engine::components::EnabledComponent>(entity))
        {
            auto& en = registry.GetComponent<engine::components::EnabledComponent>(entity);
            if (ImGui::CollapsingHeader("Enabled"))
            {
                ImGui::Checkbox("Enabled", &en.IsEnabled);
            }
        }

        // Static
        if (registry.HasComponent<engine::components::StaticComponent>(entity))
        {
            auto& st = registry.GetComponent<engine::components::StaticComponent>(entity);
            if (ImGui::CollapsingHeader("Static"))
            {
                ImGui::Checkbox("Static", &st.IsStatic);
            }
        }

        // Mesh
        if (registry.HasComponent<engine::components::MeshComponent>(entity))
        {
            auto& mesh = registry.GetComponent<engine::components::MeshComponent>(entity);
            if (ImGui::CollapsingHeader("Mesh"))
            {
                const char* typeNames[] = {"None", "Custom", "Cube", "Sphere", "Plane", "Quad"};
                int typeIdx = static_cast<int>(mesh.Type);
                if (ImGui::Combo("Type", &typeIdx, typeNames, 6))
                    mesh.Type = static_cast<engine::components::MeshType>(typeIdx);
                ImGui::Checkbox("Cast Shadows", &mesh.CastShadows);
            }
        }

        // Camera
        if (registry.HasComponent<engine::components::CameraComponent>(entity))
        {
            auto& cam = registry.GetComponent<engine::components::CameraComponent>(entity);
            if (ImGui::CollapsingHeader("Camera"))
            {
                const char* projNames[] = {"Perspective", "Orthographic"};
                int projIdx = static_cast<int>(cam.Projection);
                if (ImGui::Combo("Projection", &projIdx, projNames, 2))
                    cam.Projection = static_cast<engine::components::ProjectionMode>(projIdx);

                ImGui::DragFloat("FOV", &cam.FieldOfView, 0.5f, 1.0f, 179.0f);
                ImGui::DragFloat("Near Clip", &cam.NearClip, 0.01f, 0.001f, 10.0f);
                ImGui::DragFloat("Far Clip", &cam.FarClip, 1.0f, 10.0f, 10000.0f);
                ImGui::DragFloat("Ortho Size", &cam.OrthoSize, 0.1f);
                ImGui::Checkbox("Primary", &cam.Primary);
            }
        }

        // Light
        if (registry.HasComponent<engine::components::LightComponent>(entity))
        {
            auto& light = registry.GetComponent<engine::components::LightComponent>(entity);
            if (ImGui::CollapsingHeader("Light"))
            {
                const char* lightNames[] = {"None", "Directional", "Point", "Spot"};
                int lightIdx = static_cast<int>(light.Type);
                if (ImGui::Combo("Type", &lightIdx, lightNames, 4))
                    light.Type = static_cast<engine::components::LightType>(lightIdx);

                ImGui::ColorEdit3("Color", &light.Color.x);
                ImGui::DragFloat("Intensity", &light.Intensity, 0.1f);
                ImGui::DragFloat("Range", &light.Range, 0.1f);
                ImGui::Checkbox("Cast Shadows", &light.CastShadows);
            }
        }

        // Add Component button
        if (ImGui::Button("Add Component", ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            if (ImGui::MenuItem("Mesh"))
                registry.AddComponent<engine::components::MeshComponent>(entity);
            if (ImGui::MenuItem("Camera"))
                registry.AddComponent<engine::components::CameraComponent>(entity);
            if (ImGui::MenuItem("Light"))
                registry.AddComponent<engine::components::LightComponent>(entity);
            if (ImGui::MenuItem("Visibility"))
                registry.AddComponent<engine::components::VisibilityComponent>(entity);
            if (ImGui::MenuItem("Static"))
                registry.AddComponent<engine::components::StaticComponent>(entity);
            ImGui::EndPopup();
        }
    }

} // namespace editor
