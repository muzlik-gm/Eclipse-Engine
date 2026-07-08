// ============================================================================
// File: Editor/Source/Panels/ScenePanel.cpp
// ============================================================================
#include "Editor/Panels/ScenePanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Camera/EditorCamera.h"
#include "Editor/Gizmos/GizmoManager.h"
#include "Editor/Selection/EditorSelection.h"
#include "Editor/Theme/ThemeManager.h"
#include "Editor/Prefs/EditorPreferences.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Math/Math.h"
#include "Engine/ECS/Registry.h"

#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace editor {

    using engine::math::Vec2;
    using engine::math::Vec3;
    using engine::math::Vec4;
    using engine::math::Mat4;
    using engine::components::TransformComponent;

    ScenePanel::ScenePanel(GLFWwindow* window) : m_Window(window) {}

    void ScenePanel::OnRender(EditorContext& context)
    {
        RenderViewportToolbar(context);
        ImGui::Separator();

        RenderScene(context);
    }

    void ScenePanel::RenderViewportToolbar(EditorContext& context)
    {
        auto& gizmos = context.GetGizmos();

        if (ImGui::Button("Move (W)"))
            gizmos.SetMode(GizmoMode::Translate);
        ImGui::SameLine();
        if (ImGui::Button("Rotate (E)"))
            gizmos.SetMode(GizmoMode::Rotate);
        ImGui::SameLine();
        if (ImGui::Button("Scale (R)"))
            gizmos.SetMode(GizmoMode::Scale);

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        bool local = (gizmos.GetSpace() == GizmoSpace::Local);
        if (ImGui::Checkbox("Local", &local))
            gizmos.SetSpace(local ? GizmoSpace::Local : GizmoSpace::World);

        ImGui::SameLine();

        bool snap = gizmos.IsSnappingEnabled();
        if (ImGui::Checkbox("Snap", &snap))
            gizmos.SetSnapping(snap);
    }

    void ScenePanel::RenderScene(EditorContext& context)
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x <= 0 || size.y <= 0)
            return;

        // Resize the framebuffer if needed.
        u32 w = static_cast<u32>(size.x);
        u32 h = static_cast<u32>(size.y);
        if (m_Framebuffer.NeedsResize(w, h) || !m_Framebuffer.IsValid())
        {
            m_Framebuffer.Resize(w, h);
            context.GetCamera().SetViewportSize(w, h);
            ENGINE_LOG_DEBUG("ScenePanel — resized framebuffer to {}x{} (valid={})",
                             w, h, m_Framebuffer.IsValid());
        }

        // Don't render scene or display image if framebuffer is invalid.
        if (!m_Framebuffer.IsValid())
        {
            // Draw a placeholder so the panel isn't blank.
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max = ImVec2(p_min.x + size.x, p_min.y + size.y);
            ImU32 col = IM_COL32(20, 20, 25, 255);
            dl->AddRectFilled(p_min, p_max, col);
            return;
        }

        // Update camera.
        auto& cam = context.GetCamera();

        // Render the scene into the framebuffer.
        // This happens INSIDE ImGui's frame, but the SceneRenderer uses
        // a GLStateSaver to save/restore all GL state.
        Mat4 viewProjection = cam.GetProjectionMatrix() * cam.GetViewMatrix();
        m_SceneRenderer.RenderScene(context, m_Framebuffer, viewProjection,
                                     context.GetPreferences().GridVisible);

        // Periodic GL error check — every ~1000 frames to avoid perf hit.
        static u32 glCheckCounter = 0;
        if (++glCheckCounter >= 1000)
        {
            glCheckCounter = 0;
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
                ENGINE_LOG_WARN("ScenePanel — GL error 0x{:X} after scene render", err);
        }

        // Display the framebuffer texture via ImGui.
        // ImGui::Image will add a draw command that samples from the texture.
        // The actual rendering happens later in ImGui_ImplOpenGL3_RenderDrawData.
        ImGui::Image(static_cast<ImTextureID>(m_Framebuffer.GetColorTextureID()),
                     size, ImVec2(0, 1), ImVec2(1, 0));

        // Capture hover/focus.
        m_IsFocused = ImGui::IsWindowFocused();
        m_IsHovered = ImGui::IsWindowHovered();

        // Handle mouse input for camera.
        ImVec2 imagePos = ImGui::GetItemRectMin();
        ImVec2 imageSize = ImGui::GetItemRectSize();
        HandleMouseInput(context, imagePos, imageSize);

        // F key: focus camera on selected entity (single-press via press/release tracking).
        if (m_IsFocused || m_IsHovered)
        {
            static bool fWasDown = false;
            auto* win = m_Window;
            bool fDown = win && glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS;
            if (fDown && !fWasDown && !m_IsFlying)
            {
                auto entity = context.GetSelection().GetPrimaryEntity();
                if (entity != engine::ecs::Invalid)
                {
                    auto* scene = context.GetActiveScene();
                    if (scene)
                    {
                        auto& registry = scene->GetRegistry();
                        if (registry.HasComponent<TransformComponent>(entity))
                        {
                            auto& tf = registry.GetComponent<TransformComponent>(entity);
                            Vec3 pos(tf.WorldMatrix[3][0], tf.WorldMatrix[3][1], tf.WorldMatrix[3][2]);
                            cam.Focus(pos, 5.0f);
                        }
                    }
                }
            }
            fWasDown = fDown;
        }

        // Selection outline and gizmo rendering moved into SceneRenderer::RenderScene,
        // inside the GLStateSaver scope so they draw into the FBO.
    }

    void ScenePanel::HandleMouseInput(EditorContext& context, ImVec2 imagePos, ImVec2 imageSize)
    {
        auto& cam = context.GetCamera();
        auto& io = ImGui::GetIO();

        // Get mouse position relative to viewport.
        ImVec2 mousePos = ImGui::GetMousePos();
        f32 mx = mousePos.x - imagePos.x;
        f32 my = mousePos.y - imagePos.y;
        bool inside = mx >= 0 && mx < imageSize.x && my >= 0 && my < imageSize.y;
        f32 vpW = imageSize.x;
        f32 vpH = imageSize.y;

        // --- Fly mode handling (ALWAYS processed, regardless of hover) ---
        // Exit on right-click release.
        if (m_IsFlying && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            if (m_Window)
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cam.SetMode(EditorCameraMode::None);
            m_IsFlying = false;
        }

        // Enter on right-click inside viewport.
        if (!m_IsFlying && inside && m_IsHovered
            && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.KeyAlt)
        {
            cam.SetMode(EditorCameraMode::Fly);
            if (m_Window)
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_IsFlying = true;
        }

        // Process fly mode (remains active even if cursor leaves viewport).
        if (m_IsFlying)
        {
            auto* win = m_Window;
            cam.SetKey(GLFW_KEY_W, win && glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS);
            cam.SetKey(GLFW_KEY_S, win && glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS);
            cam.SetKey(GLFW_KEY_A, win && glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS);
            cam.SetKey(GLFW_KEY_D, win && glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS);
            cam.SetKey(GLFW_KEY_E, win && glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);
            cam.SetKey(GLFW_KEY_Q, win && glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS);

            ImVec2 md = io.MouseDelta;
            if (md.x != 0.0f || md.y != 0.0f)
                cam.RotateView(md.x * 0.15f, md.y * 0.15f);

            return; // No other input while flying.
        }

        // --- Gizmo drag interaction ---
        if (m_GizmoDragAxis >= 0)
        {
            // End gizmo drag on left-click release.
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                m_GizmoDragAxis = -1;
            }
            else
            {
                auto* scene = context.GetActiveScene();
                auto entity = context.GetSelection().GetPrimaryEntity();
                if (scene && entity != engine::ecs::Invalid)
                {
                    auto& registry = scene->GetRegistry();
                    if (registry.HasComponent<TransformComponent>(entity))
                    {
                        auto& tf = registry.GetComponent<TransformComponent>(entity);

                        // Compute depth of gizmo origin for screen-to-world.
                        Vec4 clipPos = cam.GetProjectionMatrix() * cam.GetViewMatrix()
                                       * Vec4(m_GizmoDragOrigin.x, m_GizmoDragOrigin.y,
                                              m_GizmoDragOrigin.z, 1.0f);
                        f32 depth = (clipPos.w != 0.0f) ? clipPos.z / clipPos.w : 0.0f;

                        // Current mouse NDC.
                        Vec3 cursorNDC(
                            (mx / vpW) * 2.0f - 1.0f,
                            (my / vpH) * 2.0f - 1.0f,
                            depth);

                        Vec4 worldNear = m_GizmoDragInvVP * Vec4(cursorNDC.x, cursorNDC.y, -1.0f, 1.0f);
                        Vec4 worldFar  = m_GizmoDragInvVP * Vec4(cursorNDC.x, cursorNDC.y,  1.0f, 1.0f);
                        Vec3 nearPos = Vec3(worldNear) / worldNear.w;
                        Vec3 farPos  = Vec3(worldFar) / worldFar.w;

                        // Ray from camera through cursor.
                        Vec3 dir = engine::math::Normalize(farPos - nearPos);

                        // Axis direction in world space (gizmo is world-aligned).
                        Vec3 axis(0.0f);
                        if (m_GizmoDragAxis == 0) axis = Vec3(1.0f, 0.0f, 0.0f);
                        else if (m_GizmoDragAxis == 1) axis = Vec3(0.0f, 1.0f, 0.0f);
                        else axis = Vec3(0.0f, 0.0f, 1.0f);

                        // Intersect ray with axis line (origin + t * axis).
                        Vec3 origin = m_GizmoDragOrigin;
                        Vec3 p = origin - nearPos;
                        f32 t = engine::math::Dot(p, axis) / engine::math::Dot(dir, axis);
                        Vec3 hit = nearPos + dir * t;

                        // Delta along axis from drag start.
                        Vec3 delta = hit - m_GizmoDragStart;
                        f32 d = engine::math::Dot(delta, axis);

                        // Apply to translation.
                        if (context.GetGizmos().GetMode() == GizmoMode::Translate)
                        {
                            tf.Translation += axis * d;
                            m_GizmoDragStart += axis * d;
                        }
                    }
                }
            }
            return;
        }

        // --- Standard input (requires hover) ---
        if (!m_IsHovered || !inside)
            return;

        // Gizmo axis picking via left-click.
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyCtrl)
        {
            // --- 1) Try to grab the gizmo of the currently selected entity ---
            // We test screen-space proximity to the gizmo axes FIRST so the user
            // does not have to hit the thin 1px gizmo line in the pick buffer
            // (missing it would just clear the selection and prevent dragging).
            auto selectedEntity = context.GetSelection().GetPrimaryEntity();
            bool startedGizmoDrag = false;
            if (selectedEntity != engine::ecs::Invalid
                && context.GetGizmos().GetMode() != GizmoMode::None)
            {
                auto* scene = context.GetActiveScene();
                if (scene && scene->GetRegistry().HasComponent<TransformComponent>(selectedEntity))
                {
                    auto& tf = scene->GetRegistry()
                                   .GetComponent<TransformComponent>(selectedEntity);
                    Vec3 pos(tf.WorldMatrix[3][0], tf.WorldMatrix[3][1], tf.WorldMatrix[3][2]);

                    // Compute gizmo scale (must match RenderGizmos).
                    f32 dist = engine::math::Length(pos - cam.GetPosition());
                    f32 scale = std::max(dist * 0.15f, 0.5f);

                    Mat4 viewProj = cam.GetProjectionMatrix() * cam.GetViewMatrix();

                    Vec3 axisDirs[3] = {
                        Vec3(scale, 0.0f, 0.0f),
                        Vec3(0.0f, scale, 0.0f),
                        Vec3(0.0f, 0.0f, scale)
                    };

                    float bestDist = 14.0f; // Pixel threshold.
                    int bestAxis = -1;
                    for (int a = 0; a < 3; ++a)
                    {
                        Vec4 orgClip = viewProj * Vec4(pos, 1.0f);
                        Vec4 endClip = viewProj * Vec4(pos + axisDirs[a], 1.0f);
                        if (orgClip.w <= 0.0f || endClip.w <= 0.0f) continue;

                        Vec2 orgScreen(
                            (orgClip.x / orgClip.w) * 0.5f + 0.5f,
                            (orgClip.y / orgClip.w) * 0.5f + 0.5f);
                        Vec2 endScreen(
                            (endClip.x / endClip.w) * 0.5f + 0.5f,
                            (endClip.y / endClip.w) * 0.5f + 0.5f);
                        orgScreen.x *= vpW; orgScreen.y *= vpH;
                        endScreen.x *= vpW; endScreen.y *= vpH;

                        // Distance from cursor to the axis line segment.
                        Vec2 ab = endScreen - orgScreen;
                        float abLen2 = ab.x * ab.x + ab.y * ab.y;
                        float t = 0.0f;
                        if (abLen2 > 1e-6f)
                        {
                            t = ((mx - orgScreen.x) * ab.x + (my - orgScreen.y) * ab.y) / abLen2;
                            t = std::max(0.0f, std::min(1.0f, t));
                        }
                        Vec2 closest = orgScreen + ab * t;
                        float dx = mx - closest.x;
                        float dy = my - closest.y;
                        float d = std::sqrt(dx * dx + dy * dy);

                        if (d < bestDist)
                        {
                            bestDist = d;
                            bestAxis = a;
                        }
                    }

                    if (bestAxis >= 0)
                    {
                        m_GizmoDragAxis = bestAxis;
                        m_GizmoDragOrigin = pos;
                        m_GizmoDragStart = pos;
                        m_GizmoDragInvVP = glm::inverse(viewProj);
                        startedGizmoDrag = true;
                    }
                }
            }

            if (startedGizmoDrag)
                return;

            // --- 2) Otherwise do normal entity picking ---
            GLint lastFBO = 0;
            GLint lastViewport[4] = {0, 0, 0, 0};
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);
            glGetIntegerv(GL_VIEWPORT, lastViewport);

            Mat4 vp = cam.GetProjectionMatrix() * cam.GetViewMatrix();
            m_Picking.RenderPickBuffer(m_Framebuffer, m_SceneRenderer, context, vp);

            u32 px = static_cast<u32>(mx);
            u32 py = static_cast<u32>(my);
            auto entity = m_Picking.PickAt(px, py);

            glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
            glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);

            if (entity != engine::ecs::Invalid)
                context.GetSelection().SelectEntity(entity);
            else
                context.GetSelection().Clear();

            return;
        }

        // Orbit/pan on middle-mouse or right-mouse drag.
        {
            ImGuiMouseButton dragBtn = ImGuiMouseButton_COUNT;
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
                dragBtn = ImGuiMouseButton_Middle;
            else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right) && !io.KeyAlt)
                dragBtn = ImGuiMouseButton_Right;

            if (dragBtn != ImGuiMouseButton_COUNT)
            {
                ImVec2 delta = ImGui::GetMouseDragDelta(dragBtn);
                if (delta.x != 0.0f || delta.y != 0.0f)
                {
                    if (io.KeyShift)
                        cam.Pan(delta.x * 0.5f, delta.y * 0.5f);
                    else
                        cam.Orbit(delta.x * 0.5f, delta.y * 0.5f);
                    ImGui::ResetMouseDragDelta(dragBtn);
                }
            }
        }

        // Zoom with mouse wheel.
        f32 wheel = io.MouseWheel;
        if (wheel != 0.0f)
            cam.Zoom(-wheel);
    }

} // namespace editor
