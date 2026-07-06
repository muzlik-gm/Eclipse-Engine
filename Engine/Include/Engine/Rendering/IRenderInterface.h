// ============================================================================
// File: Engine/Include/Engine/Rendering/IRenderInterface.h
// Abstract interface for submitting renderable data from World to Renderer.
// ============================================================================
#pragma once

#include "Engine/ECS/Entity.h"
#include "Engine/Core/Types.h"

namespace engine::rendering {

    using engine::ecs::Entity;

    /// @brief Minimal renderable data packet extracted from World entities.
    struct RenderSubmitInfo
    {
        Entity Entity;
        void* MeshData{nullptr};
        void* TransformData{nullptr};
        void* CameraData{nullptr};
        bool  IsVisible{true};
    };

    /// @brief Abstract interface for the renderer to receive renderable data.
    class IRenderInterface
    {
    public:
        virtual ~IRenderInterface() = default;

        virtual void SubmitRenderable(const RenderSubmitInfo& info) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void SetActiveCamera(const RenderSubmitInfo& cameraInfo) = 0;
    };

} // namespace engine::rendering
