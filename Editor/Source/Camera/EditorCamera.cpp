// ============================================================================
// File: Editor/Source/Camera/EditorCamera.cpp
// ============================================================================
#include "Editor/Camera/EditorCamera.h"
#include "Engine/Math/Math.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

namespace editor {

    EditorCamera::EditorCamera()
    {
        UpdateVectors();
    }

    void EditorCamera::Update(f64 deltaTime)
    {
        // Handle fly mode with WASD.
        if (m_Mode == EditorCameraMode::Fly)
        {
            f32 forward = 0.0f, right = 0.0f, up = 0.0f;
            if (m_Keys[GLFW_KEY_W]) forward += 1.0f;
            if (m_Keys[GLFW_KEY_S]) forward -= 1.0f;
            if (m_Keys[GLFW_KEY_D]) right += 1.0f;
            if (m_Keys[GLFW_KEY_A]) right -= 1.0f;
            if (m_Keys[GLFW_KEY_E]) up += 1.0f;
            if (m_Keys[GLFW_KEY_Q]) up -= 1.0f;

            if (forward != 0.0f || right != 0.0f || up != 0.0f)
                Fly(forward, right, up, deltaTime);
        }
    }

    const Mat4& EditorCamera::GetViewMatrix() const
    {
        if (m_ViewDirty)
            RecalculateView();
        return m_ViewMatrix;
    }

    const Mat4& EditorCamera::GetProjectionMatrix() const
    {
        if (m_ProjDirty)
            RecalculateProjection();
        return m_ProjectionMatrix;
    }

    Mat4 EditorCamera::GetViewProjectionMatrix() const
    {
        return GetProjectionMatrix() * GetViewMatrix();
    }

    void EditorCamera::Orbit(f32 deltaX, f32 deltaY)
    {
        // Spherical coordinates orbit around the target.
        f32 azimuth = std::atan2(m_Position.x - m_Target.x, m_Position.z - m_Target.z);
        f32 elevation = std::asin((m_Position.y - m_Target.y) / m_Distance);

        azimuth -= deltaX * m_RotateSpeed * 0.01f;
        elevation += deltaY * m_RotateSpeed * 0.01f;

        // Clamp elevation to avoid gimbal lock.
        elevation = std::clamp(elevation, -1.55f, 1.55f);

        m_Position.x = m_Target.x + m_Distance * std::cos(elevation) * std::sin(azimuth);
        m_Position.y = m_Target.y + m_Distance * std::sin(elevation);
        m_Position.z = m_Target.z + m_Distance * std::cos(elevation) * std::cos(azimuth);

        m_ViewDirty = true;
        UpdateVectors();
    }

    void EditorCamera::Pan(f32 deltaX, f32 deltaY)
    {
        f32 panAmount = m_PanSpeed * m_Distance;
        Vec3 offset = m_Right * (-deltaX * panAmount) + m_Up * (deltaY * panAmount);
        m_Position += offset;
        m_Target += offset;
        m_ViewDirty = true;
    }

    void EditorCamera::Zoom(f32 delta)
    {
        m_Distance *= (1.0f + delta * m_ZoomSpeed * 0.1f);
        m_Distance = std::max(m_Distance, 0.1f);

        // Move position toward/away from target.
        Vec3 dir = m_Position - m_Target;
        dir = engine::math::Normalize(dir);
        m_Position = m_Target + dir * m_Distance;
        m_ViewDirty = true;
    }

    void EditorCamera::Fly(f32 forward, f32 right, f32 up, f64 deltaTime)
    {
        f32 speed = m_MoveSpeed * static_cast<f32>(deltaTime);
        Vec3 move = (m_Forward * forward + m_Right * right + m_Up * up) * speed;
        m_Position += move;
        m_Target += move;
        m_ViewDirty = true;
    }

    void EditorCamera::Focus(const Vec3& target, f32 distance)
    {
        m_Target = target;
        if (distance > 0.0f)
            m_Distance = distance;

        Vec3 dir = m_Position - m_Target;
        if (engine::math::Length(dir) < 0.001f)
            dir = Vec3(0.0f, 0.0f, 1.0f);
        dir = engine::math::Normalize(dir);
        m_Position = m_Target + dir * m_Distance;
        m_ViewDirty = true;
        UpdateVectors();
    }

    void EditorCamera::SetMouseButton(u32 button, bool pressed)
    {
        if (button < 5)
            m_MouseButtons[button] = pressed;
    }

    void EditorCamera::SetKey(u32 key, bool pressed)
    {
        if (key < 512)
            m_Keys[key] = pressed;
    }

    void EditorCamera::UpdateVectors()
    {
        m_Forward = engine::math::Normalize(m_Target - m_Position);
        m_Right = engine::math::Normalize(engine::math::Cross(m_Forward, Vec3(0.0f, 1.0f, 0.0f)));
        m_Up = engine::math::Cross(m_Right, m_Forward);
    }

    void EditorCamera::RecalculateView() const
    {
        m_ViewMatrix = engine::math::LookAt(m_Position, m_Target, Vec3(0.0f, 1.0f, 0.0f));
        m_ViewDirty = false;
    }

    void EditorCamera::RecalculateProjection() const
    {
        m_ProjectionMatrix = engine::math::Perspective(
            engine::math::Radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
        m_ProjDirty = false;
    }

} // namespace editor
