// ============================================================================
// File: Editor/Include/Editor/Viewport/FramebufferManager.h
// Manages all editor framebuffers (Scene View, Game View, etc.)
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Editor/Viewport/ViewportFramebuffer.h"

#include <memory>
#include <unordered_map>
#include <string>

namespace editor {

    /// @brief Manages named framebuffers for the editor.  Each viewport
    ///        (Scene View, Game View) registers its framebuffer here so
    ///        that other systems (e.g. picking, debug rendering) can
    ///        access the right framebuffer.
    class FramebufferManager
    {
    public:
        FramebufferManager() = default;
        ~FramebufferManager() = default;

        /// @brief Creates or retrieves a framebuffer by name.
        ViewportFramebuffer& GetFramebuffer(const std::string& name);

        /// @brief Returns the framebuffer with @p name, or nullptr.
        [[nodiscard]] ViewportFramebuffer* Find(const std::string& name);

        /// @brief Returns the framebuffer with @p name, or nullptr.
        [[nodiscard]] const ViewportFramebuffer* Find(const std::string& name) const;

        /// @brief Resizes all framebuffers to match their requested sizes.
        void UpdateAll();

        /// @brief Clears all framebuffers.
        void ClearAll();

        /// @brief Resizes a specific framebuffer.
        void Resize(const std::string& name, engine::core::u32 width, engine::core::u32 height);

    private:
        std::unordered_map<std::string, std::unique_ptr<ViewportFramebuffer>> m_Framebuffers;
    };

} // namespace editor
