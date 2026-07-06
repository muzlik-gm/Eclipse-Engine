// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLContext.h
// OpenGL rendering context backed by GLFW.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsDevice.h"

struct GLFWwindow;

namespace engine::opengl {

    class OpenGLDebugLayer;

    class OpenGLContext final : public engine::rhi::IGraphicsContext
    {
    public:
        OpenGLContext();
        ~OpenGLContext() override;

        bool Create(GLFWwindow* window, bool enableDebug);
        void Destroy();

        // -- IGraphicsObject
        std::string_view GetDebugName() const noexcept override { return "OpenGLContext"; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override;
        bool IsValid() const noexcept override { return m_Window != nullptr; }

        // -- IGraphicsContext
        void MakeCurrent() override;
        void ReleaseCurrent() override;
        bool IsCurrent() const noexcept override;
        void SwapBuffers() override;
        void SetVSync(bool enabled) override;
        bool IsVSyncEnabled() const noexcept override { return m_VSync; }

        [[nodiscard]] GLFWwindow* GetWindow() const noexcept { return m_Window; }

    private:
        GLFWwindow* m_Window{nullptr};
        bool        m_VSync{true};
        bool        m_OwnsWindow{false};
    };

} // namespace engine::opengl
