// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLBackend.h
// ============================================================================
#pragma once

#include "Engine/Renderer/RHI/Factories/IGraphicsBackend.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsDevice.h"

#include <memory>

namespace engine::opengl {

    class OpenGLBackend final : public engine::rhi::IGraphicsBackend
    {
    public:
        OpenGLBackend();
        ~OpenGLBackend() override = default;

        engine::rhi::GraphicsBackend GetType() const noexcept override
        { return engine::rhi::GraphicsBackend::OpenGL; }

        std::string_view GetName() const noexcept override
        { return "OpenGL 4.6 Core"; }

        bool IsAvailable() const override;

        std::unique_ptr<engine::rhi::IGraphicsInstance>
            CreateInstance(const engine::rhi::BackendCreateInfo& info) override;
    };

} // namespace engine::opengl
