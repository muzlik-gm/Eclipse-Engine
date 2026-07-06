// ============================================================================
// File: Editor/Source/Viewport/FramebufferManager.cpp
// ============================================================================
#include "Editor/Viewport/FramebufferManager.h"
#include "Engine/Core/Log.h"

namespace editor {

    ViewportFramebuffer& FramebufferManager::GetFramebuffer(const std::string& name)
    {
        auto it = m_Framebuffers.find(name);
        if (it == m_Framebuffers.end())
        {
            auto fb = std::make_unique<ViewportFramebuffer>();
            auto* raw = fb.get();
            m_Framebuffers[name] = std::move(fb);
            return *raw;
        }
        return *it->second;
    }

    ViewportFramebuffer* FramebufferManager::Find(const std::string& name)
    {
        auto it = m_Framebuffers.find(name);
        return (it != m_Framebuffers.end()) ? it->second.get() : nullptr;
    }

    const ViewportFramebuffer* FramebufferManager::Find(const std::string& name) const
    {
        auto it = m_Framebuffers.find(name);
        return (it != m_Framebuffers.end()) ? it->second.get() : nullptr;
    }

    void FramebufferManager::UpdateAll()
    {
        // Framebuffers resize themselves via Resize() calls from panels.
    }

    void FramebufferManager::ClearAll()
    {
        m_Framebuffers.clear();
    }

    void FramebufferManager::Resize(const std::string& name, engine::core::u32 width,
                                     engine::core::u32 height)
    {
        GetFramebuffer(name).Resize(width, height);
    }

} // namespace editor
