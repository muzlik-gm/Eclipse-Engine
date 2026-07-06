// ============================================================================
// File: Engine/Include/Engine/Renderer/WorldRenderer.h
// Bridges the World (ECS) to the Renderer — extracts renderable data from
// entities and submits it through the RHI.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Renderer/RHI/RHI.h"
#include "Engine/Renderer/Core/Renderer.h"

namespace engine::scene { class Scene; }

namespace engine::renderer {

    using engine::ecs::Entity;
    using engine::math::Mat4;

    // ========================================================================
    // RenderableData — extracted from entities for rendering.
    // ========================================================================

    /// @brief Renderable data extracted from a single entity.
    struct RenderableData
    {
        ecs::Entity       Entity{ecs::Invalid};
        math::Mat4        WorldMatrix{1.0f};
        math::Mat4        ViewMatrix{1.0f};
        math::Mat4        ProjectionMatrix{1.0f};
        components::MeshType  MeshType{components::MeshType::None};
        bool              IsVisible{true};
        bool              IsCamera{false};
        bool              IsPrimaryCamera{false};
    };

    // ========================================================================
    // WorldRenderer — submits renderable entities to the Renderer.
    // ========================================================================

    /// @brief Bridges the ECS World to the backend-independent Renderer.
    ///
    /// Each frame, the WorldRenderer iterates the active Scene's registry,
    /// extracts renderable data (MeshComponent + TransformComponent) and
    /// camera data (CameraComponent + TransformComponent), and submits
    /// them to the Renderer's command buffer via the RHI.
    ///
    /// The WorldRenderer does NOT own any GPU resources — it uses the
    /// RendererContext's factory to create temporary procedural geometry
    /// for validation.  Future phases will replace this with the asset
    /// pipeline.
    class WorldRenderer
    {
    public:
        WorldRenderer() = default;
        ~WorldRenderer() = default;

        /// @brief Initializes the WorldRenderer with a Renderer context.
        ///        Creates temporary procedural geometry for validation.
        bool Initialize(RendererContext& context);

        /// @brief Shuts down the WorldRenderer, releasing temporary resources.
        void Shutdown();

        /// @brief Extracts renderable data from @p scene and submits it
        ///        to the Renderer's command buffer.
        void RenderScene(scene::Scene& scene);

        /// @brief Returns the number of renderables submitted last frame.
        [[nodiscard]] core::u32 GetRenderableCount() const noexcept
        { return m_RenderableCount; }

        /// @brief Returns the number of cameras found last frame.
        [[nodiscard]] core::u32 GetCameraCount() const noexcept
        { return m_CameraCount; }

    private:
        void CreateProceduralGeometry();
        void DestroyProceduralGeometry();

        RendererContext*                               m_Context{nullptr};
        core::u32                                      m_RenderableCount{0};
        core::u32                                      m_CameraCount{0};

        // Temporary procedural geometry for validation.
        std::unique_ptr<rhi::IVertexBuffer>            m_TriangleVB;
        std::unique_ptr<rhi::IIndexBuffer>             m_TriangleIB;
        std::unique_ptr<rhi::IVertexArray>             m_TriangleVAO;
        std::unique_ptr<rhi::IShader>                  m_ValidationShader;
        std::unique_ptr<rhi::IGraphicsPipeline>        m_ValidationPipeline;

        bool                                           m_Initialized{false};
    };

} // namespace engine::renderer
