// ============================================================================
// File: Editor/Include/Editor/Picking/EntityPicking.h
// GPU-based entity picking via ID rendering.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Math/Math.h"

#include <glad/glad.h>
#include <memory>

namespace editor {

    class ViewportFramebuffer;

    /// @brief GPU-based entity picking.  Renders each entity with a unique
    ///        color ID into a pick buffer, then reads back the pixel under
    ///        the mouse cursor to determine which entity was clicked.
    class EntityPicking
    {
    public:
        EntityPicking();
        ~EntityPicking();

        /// @brief Renders the pick buffer for the given entities.
        ///        Each entity is rendered with a unique color derived
        ///        from its entity index.
        /// @param framebuffer The viewport framebuffer to render into.
        /// @param viewProjection The view-projection matrix.
        void RenderPickBuffer(ViewportFramebuffer& framebuffer,
                              const engine::math::Mat4& viewProjection);

        /// @brief Reads the pixel at @p x, @p y and returns the picked
        ///        entity.  Returns Invalid if no entity was clicked.
        /// @param x Mouse X in viewport coordinates.
        /// @param y Mouse Y in viewport coordinates (top-left origin).
        [[nodiscard]] engine::ecs::Entity PickAt(engine::core::u32 x, engine::core::u32 y);

        /// @brief Returns the entity index encoded as a color for @p entity.
        [[nodiscard]] static engine::math::Vec3 EncodeEntityID(engine::ecs::Entity entity) noexcept;

        /// @brief Decodes a pixel color back into an entity index.
        [[nodiscard]] static engine::core::u32 DecodeEntityID(
            engine::core::u8 r, engine::core::u8 g, engine::core::u8 b) noexcept;

    private:
        GLuint m_PickFBO{0};
        GLuint m_PickTexture{0};
        GLuint m_PickDepth{0};
        engine::core::u32 m_Width{0};
        engine::core::u32 m_Height{0};

        void EnsureSize(engine::core::u32 width, engine::core::u32 height);
        void Destroy();
    };

} // namespace editor
