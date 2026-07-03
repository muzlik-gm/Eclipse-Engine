// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWClipboard.cpp
// GLFW-backed implementation of the IClipboard interface.
// ============================================================================

#include "Engine/Platform/Clipboard.h"
#include "Engine/Core/Log.h"

#include <GLFW/glfw3.h>

namespace engine::platform
{

    // ========================================================================
    // GLFWClipboard
    // ========================================================================

    class GLFWClipboard final : public IClipboard
    {
    public:
        explicit GLFWClipboard(GLFWwindow* window)
            : m_Window(window)
        {}

        void SetText(const std::string& text) override
        {
            if (m_Window)
            {
                glfwSetClipboardString(m_Window, text.c_str());
            }
        }

        [[nodiscard]] std::string GetText() const override
        {
            if (!m_Window) return "";
            const char* text = glfwGetClipboardString(m_Window);
            return text ? std::string(text) : "";
        }

        void Clear() override
        {
            if (m_Window)
            {
                glfwSetClipboardString(m_Window, "");
            }
        }

        [[nodiscard]] bool HasText() const override
        {
            if (!m_Window) return false;
            const char* text = glfwGetClipboardString(m_Window);
            return text != nullptr && text[0] != '\0';
        }

    private:
        GLFWwindow* m_Window = nullptr;
    };

    // ========================================================================
    // IClipboard factory
    // ========================================================================

    std::unique_ptr<IClipboard> IClipboard::Create()
    {
        return std::make_unique<GLFWClipboard>(nullptr);
    }

} // namespace engine::platform
