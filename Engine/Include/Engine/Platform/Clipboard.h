// ============================================================================
// File: Engine/Include/Engine/Platform/Clipboard.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <memory>
#include <string>

namespace engine::platform {

    // ========================================================================
    // IClipboard – pure-virtual clipboard interface
    // ========================================================================
    class IClipboard : private engine::core::NonCopyable
    {
    public:
        virtual ~IClipboard() = default;

        virtual void SetText(const std::string& text) = 0;
        [[nodiscard]] virtual std::string GetText() const = 0;
        virtual void Clear() = 0;
        [[nodiscard]] virtual bool HasText() const = 0;

        // --------------------------------------------------------------------
        // Factory – creates the platform-specific implementation.
        // --------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<IClipboard> Create();
    };

} // namespace engine::platform