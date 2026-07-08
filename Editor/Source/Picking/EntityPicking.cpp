// ============================================================================
// File: Editor/Source/Picking/EntityPicking.cpp
// ============================================================================
#include "Editor/Picking/EntityPicking.h"
#include "Editor/Rendering/SceneRenderer.h"
#include "Editor/Viewport/ViewportFramebuffer.h"
#include "Engine/Core/Log.h"

namespace editor {

    using engine::core::u32;
    using engine::core::u8;

    EntityPicking::EntityPicking() = default;

    EntityPicking::~EntityPicking()
    {
        Destroy();
    }

    void EntityPicking::EnsureSize(u32 width, u32 height)
    {
        if (m_Width == width && m_Height == height && m_PickFBO != 0)
            return;

        Destroy();

        m_Width = width;
        m_Height = height;

        glGenFramebuffers(1, &m_PickFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_PickFBO);

        glGenTextures(1, &m_PickTexture);
        glBindTexture(GL_TEXTURE_2D, m_PickTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, m_PickTexture, 0);

        glGenTextures(1, &m_PickDepth);
        glBindTexture(GL_TEXTURE_2D, m_PickDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D, m_PickDepth, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void EntityPicking::Destroy()
    {
        if (m_PickDepth)   { glDeleteTextures(1, &m_PickDepth);   m_PickDepth = 0; }
        if (m_PickTexture) { glDeleteTextures(1, &m_PickTexture); m_PickTexture = 0; }
        if (m_PickFBO)     { glDeleteFramebuffers(1, &m_PickFBO); m_PickFBO = 0; }
        m_Width = 0;
        m_Height = 0;
    }

    void EntityPicking::RenderPickBuffer(ViewportFramebuffer& framebuffer,
                                          SceneRenderer& renderer,
                                          EditorContext& context,
                                          const engine::math::Mat4& viewProjection)
    {
        // Save GL state.
        GLint lastFBO = 0;
        GLint lastViewport[4] = {0, 0, 0, 0};
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);
        glGetIntegerv(GL_VIEWPORT, lastViewport);

        EnsureSize(framebuffer.GetWidth(), framebuffer.GetHeight());

        glBindFramebuffer(GL_FRAMEBUFFER, m_PickFBO);
        glViewport(0, 0, static_cast<GLsizei>(m_Width), static_cast<GLsizei>(m_Height));

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render all entities with pick IDs into the pick FBO.
        renderer.RenderPicking(context, viewProjection);

        // Restore.
        glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
        glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
    }

    engine::ecs::Entity EntityPicking::PickAt(u32 x, u32 y)
    {
        if (m_PickFBO == 0 || x >= m_Width || y >= m_Height)
            return engine::ecs::Invalid;

        GLint lastFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

        glBindFramebuffer(GL_FRAMEBUFFER, m_PickFBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        u8 pixel[4] = {0, 0, 0, 0};
        glReadPixels(static_cast<GLint>(x), static_cast<GLint>(m_Height - y - 1),
                     1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);

        u32 entityID = DecodeEntityID(pixel[0], pixel[1], pixel[2]);
        if (entityID == 0)
            return engine::ecs::Invalid;

        return static_cast<engine::ecs::Entity>(entityID - 1);
    }

    engine::math::Vec3 EntityPicking::EncodeEntityID(engine::ecs::Entity entity) noexcept
    {
        u32 id = static_cast<u32>(entity) + 1;
        float r = static_cast<float>((id & 0x000000FF)) / 255.0f;
        float g = static_cast<float>((id & 0x0000FF00) >> 8) / 255.0f;
        float b = static_cast<float>((id & 0x00FF0000) >> 16) / 255.0f;
        return {r, g, b};
    }

    u32 EntityPicking::DecodeEntityID(u8 r, u8 g, u8 b) noexcept
    {
        return static_cast<u32>(r) | (static_cast<u32>(g) << 8) | (static_cast<u32>(b) << 16);
    }

} // namespace editor
