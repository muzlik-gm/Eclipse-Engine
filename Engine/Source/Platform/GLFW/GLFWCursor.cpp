// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWCursor.cpp
// GLFW-backed implementation of the ICursor interface.
// ============================================================================

#include "Engine/Platform/Cursor.h"
#include "Engine/Core/Log.h"

#include <GLFW/glfw3.h>

namespace engine::platform
{

    using engine::core::f64;

    // ========================================================================
    // Helper — map CursorShape to GLFW standard cursor
    // ========================================================================

    static GLFWcursor* CreateStandardCursor(CursorShape shape)
    {
        int glfwShape = GLFW_ARROW_CURSOR;
        switch (shape)
        {
            case CursorShape::Arrow:     glfwShape = GLFW_ARROW_CURSOR;     break;
            case CursorShape::IBeam:     glfwShape = GLFW_IBEAM_CURSOR;     break;
            case CursorShape::Crosshair: glfwShape = GLFW_CROSSHAIR_CURSOR; break;
            case CursorShape::Hand:      glfwShape = GLFW_HAND_CURSOR;      break;
            case CursorShape::HResize:   glfwShape = GLFW_HRESIZE_CURSOR;  break;
            case CursorShape::VResize:   glfwShape = GLFW_VRESIZE_CURSOR;  break;
            case CursorShape::ResizeAll: glfwShape = GLFW_RESIZE_ALL_CURSOR; break;
            case CursorShape::NoCursor:  return nullptr;
        }
        return glfwCreateStandardCursor(glfwShape);
    }

    // ========================================================================
    // GLFWCursor
    // ========================================================================

    class GLFWCursor final : public ICursor
    {
    public:
        explicit GLFWCursor(GLFWwindow* window)
            : m_Window(window)
        {}

        ~GLFWCursor() override
        {
            if (m_CurrentCursor)
            {
                glfwDestroyCursor(m_CurrentCursor);
            }
        }

        void SetShape(CursorShape shape) override
        {
            m_Shape = shape;

            if (m_CurrentCursor)
            {
                glfwDestroyCursor(m_CurrentCursor);
                m_CurrentCursor = nullptr;
            }

            if (shape == CursorShape::NoCursor)
            {
                if (m_Window) glfwSetCursor(m_Window, nullptr);
                return;
            }

            m_CurrentCursor = CreateStandardCursor(shape);
            if (m_CurrentCursor && m_Window)
            {
                glfwSetCursor(m_Window, m_CurrentCursor);
            }
        }

        [[nodiscard]] CursorShape GetShape() const override
        {
            return m_Shape;
        }

        [[nodiscard]] bool IsVisible() const override
        {
            return m_Visible && m_Shape != CursorShape::NoCursor;
        }

        void SetVisible(bool visible) override
        {
            m_Visible = visible;
            if (!m_Window) return;

            if (visible)
            {
                SetShape(m_Shape);
            }
            else
            {
                glfwSetCursor(m_Window, nullptr);
            }
        }

        void SetPosition(f64 x, f64 y) override
        {
            if (m_Window) glfwSetCursorPos(m_Window, x, y);
        }

        void GetPosition(f64& outX, f64& outY) const override
        {
            if (m_Window)
            {
                glfwGetCursorPos(m_Window, &outX, &outY);
                return;
            }
            outX = 0.0;
            outY = 0.0;
        }

        void EnableLockMode() override
        {
            m_Locked = true;
            if (m_Window) glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        void DisableLockMode() override
        {
            m_Locked = false;
            if (m_Window) glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        [[nodiscard]] bool IsLocked() const override
        {
            return m_Locked;
        }

    private:
        GLFWwindow* m_Window         = nullptr;
        GLFWcursor* m_CurrentCursor   = nullptr;
        CursorShape m_Shape           = CursorShape::Arrow;
        bool        m_Visible         = true;
        bool        m_Locked          = false;
    };

    // ========================================================================
    // ICursor factory
    // ========================================================================

    std::unique_ptr<ICursor> ICursor::Create(void* windowNativeHandle)
    {
        auto* glfwWindow = static_cast<GLFWwindow*>(windowNativeHandle);
        return std::make_unique<GLFWCursor>(glfwWindow);
    }

} // namespace engine::platform
