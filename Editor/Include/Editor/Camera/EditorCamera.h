// ============================================================================
// File: Editor/Include/Editor/Camera/EditorCamera.h
// Professional editor camera with orbit/pan/fly/zoom.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"

namespace editor {

    using engine::core::f32;
    using engine::core::f64;
    using engine::core::u32;
    using engine::math::Vec3;
    using engine::math::Mat4;
    using engine::math::Quat;

    // ========================================================================
    // EditorCameraMode — the current camera interaction mode.
    // ========================================================================

    enum class EditorCameraMode : u32
    {
        None  = 0,
        Orbit = 1,
        Pan   = 2,
        Fly   = 3,
        Zoom  = 4
    };

    // ========================================================================
    // EditorCamera — the viewport camera for the Scene panel.
    // ========================================================================

    /// @brief A professional editor camera that supports orbit, pan,
    ///        fly, and zoom navigation.  Designed to feel like the
    ///        cameras in Unity and Unreal Engine.
    class EditorCamera
    {
    public:
        EditorCamera();
        ~EditorCamera() = default;

        // -- Update --------------------------------------------------------

        /// @brief Updates the camera state.  Call every frame.
        /// @param deltaTime  Frame delta time in seconds.
        void Update(f64 deltaTime);

        // -- View / projection --------------------------------------------

        [[nodiscard]] const Vec3& GetPosition() const noexcept { return m_Position; }
        [[nodiscard]] const Vec3& GetTarget() const noexcept { return m_Target; }
        [[nodiscard]] const Vec3& GetUp() const noexcept { return m_Up; }
        [[nodiscard]] const Vec3& GetForward() const noexcept { return m_Forward; }
        [[nodiscard]] const Vec3& GetRight() const noexcept { return m_Right; }

        [[nodiscard]] f32 GetDistance() const noexcept { return m_Distance; }
        [[nodiscard]] f32 GetNearClip() const noexcept { return m_NearClip; }
        [[nodiscard]] f32 GetFarClip() const noexcept { return m_FarClip; }
        [[nodiscard]] f32 GetFOV() const noexcept { return m_FOV; }
        [[nodiscard]] f32 GetAspectRatio() const noexcept { return m_AspectRatio; }

        void SetPosition(const Vec3& pos) { m_Position = pos; m_ViewDirty = true; }
        void SetTarget(const Vec3& target) { m_Target = target; m_ViewDirty = true; }
        void SetDistance(f32 dist) { m_Distance = dist; m_ViewDirty = true; }
        void SetNearClip(f32 near) { m_NearClip = near; m_ProjDirty = true; }
        void SetFarClip(f32 far) { m_FarClip = far; m_ProjDirty = true; }
        void SetFOV(f32 fov) { m_FOV = fov; m_ProjDirty = true; }
        void SetAspectRatio(f32 aspect) { m_AspectRatio = aspect; m_ProjDirty = true; }
        void SetViewportSize(u32 w, u32 h)
        { m_ViewportWidth = w; m_ViewportHeight = h;
          m_AspectRatio = (h > 0) ? static_cast<f32>(w) / static_cast<f32>(h) : 1.0f;
          m_ProjDirty = true; }

        [[nodiscard]] const Mat4& GetViewMatrix() const;
        [[nodiscard]] const Mat4& GetProjectionMatrix() const;
        [[nodiscard]] Mat4 GetViewProjectionMatrix() const;

        // -- Navigation ----------------------------------------------------

        /// @brief Orbits the camera around the target.
        /// @param deltaX  Horizontal mouse delta (pixels).
        /// @param deltaY  Vertical mouse delta (pixels).
        void Orbit(f32 deltaX, f32 deltaY);

        /// @brief Rotates the view direction in place (target orbits around position).
        ///        Use this for fly-mode mouse look instead of Orbit().
        /// @param deltaX  Horizontal mouse delta (pixels).
        /// @param deltaY  Vertical mouse delta (pixels).
        void RotateView(f32 deltaX, f32 deltaY);

        /// @brief Pans the camera and target together.
        void Pan(f32 deltaX, f32 deltaY);

        /// @brief Zooms the camera toward/away from the target.
        void Zoom(f32 delta);

        /// @brief Moves the camera in fly mode using WASD-style input.
        /// @param forward  Forward input (-1, 0, +1).
        /// @param right    Right input (-1, 0, +1).
        /// @param up       Up input (-1, 0, +1).
        void Fly(f32 forward, f32 right, f32 up, f64 deltaTime);

        /// @brief Focuses the camera on a target point.
        /// @param target The point to focus on.
        /// @param distance Optional distance (0 = keep current).
        void Focus(const Vec3& target, f32 distance = 0.0f);

        // -- Settings ------------------------------------------------------

        [[nodiscard]] f32 GetMoveSpeed() const noexcept { return m_MoveSpeed; }
        void SetMoveSpeed(f32 speed) noexcept { m_MoveSpeed = speed; }

        [[nodiscard]] f32 GetRotateSpeed() const noexcept { return m_RotateSpeed; }
        void SetRotateSpeed(f32 speed) noexcept { m_RotateSpeed = speed; }

        [[nodiscard]] f32 GetZoomSpeed() const noexcept { return m_ZoomSpeed; }
        void SetZoomSpeed(f32 speed) noexcept { m_ZoomSpeed = speed; }

        [[nodiscard]] f32 GetPanSpeed() const noexcept { return m_PanSpeed; }
        void SetPanSpeed(f32 speed) noexcept { m_PanSpeed = speed; }

        // -- Input state (set by the Scene panel) -------------------------

        void SetMousePosition(f32 x, f32 y) { m_MouseX = x; m_MouseY = y; }
        void SetMouseButton(u32 button, bool pressed);
        void SetKey(u32 key, bool pressed);
        void SetMode(EditorCameraMode mode) noexcept { m_Mode = mode; }
        [[nodiscard]] EditorCameraMode GetMode() const noexcept { return m_Mode; }

    private:
        void UpdateVectors();
        void RecalculateView() const;
        void RecalculateProjection() const;

        // -- Camera state --------------------------------------------------
        Vec3   m_Position{0.0f, 5.0f, 10.0f};
        Vec3   m_Target{0.0f, 0.0f, 0.0f};
        Vec3   m_Up{0.0f, 1.0f, 0.0f};
        Vec3   m_Forward{0.0f, 0.0f, -1.0f};
        Vec3   m_Right{1.0f, 0.0f, 0.0f};
        f32    m_Distance{10.0f};

        // -- Projection ----------------------------------------------------
        f32    m_FOV{60.0f};
        f32    m_NearClip{0.1f};
        f32    m_FarClip{1000.0f};
        f32    m_AspectRatio{16.0f / 9.0f};
        u32    m_ViewportWidth{1280};
        u32    m_ViewportHeight{720};

        // -- Speeds --------------------------------------------------------
        f32    m_MoveSpeed{5.0f};
        f32    m_RotateSpeed{0.25f};
        f32    m_ZoomSpeed{1.0f};
        f32    m_PanSpeed{0.01f};

        // -- Input state ---------------------------------------------------
        f32    m_MouseX{0.0f};
        f32    m_MouseY{0.0f};
        f32    m_LastMouseX{0.0f};
        f32    m_LastMouseY{0.0f};
        bool   m_MouseButtons[5]{false, false, false, false, false};
        bool   m_Keys[512]{};
        EditorCameraMode m_Mode{EditorCameraMode::None};

        // -- Cached matrices ----------------------------------------------
        mutable Mat4 m_ViewMatrix{1.0f};
        mutable Mat4 m_ProjectionMatrix{1.0f};
        mutable bool m_ViewDirty{true};
        mutable bool m_ProjDirty{true};
    };

} // namespace editor
