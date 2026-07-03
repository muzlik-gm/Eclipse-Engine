// ============================================================================
// File: Engine/Include/Engine/Platform/OS.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <memory>
#include <string>

namespace engine::platform {

    using engine::core::f64;

    /// Severity level for OS message boxes.
    enum class MessageBoxType
    {
        Info,
        Warning,
        Error
    };

    /// Result returned after a message box is dismissed.
    struct MessageBoxResult
    {
        int ButtonClicked = 0; ///< Index of the button the user pressed.
    };

    /// Pure-virtual operating-system abstraction.
    /// Concrete implementations target each supported OS.
    class IOS
    {
    public:
        virtual ~IOS() = default;

        // --------------------------------------------------------------------
        // Clipboard
        // --------------------------------------------------------------------
        virtual void        SetClipboardText(const std::string& text) = 0;
        virtual std::string GetClipboardText() const = 0;

        // --------------------------------------------------------------------
        // Shell / UI
        // --------------------------------------------------------------------
        virtual void OpenURL(const std::string& url) = 0;
        virtual MessageBoxResult ShowMessageBox(
            const std::string& title,
            const std::string& message,
            MessageBoxType      type) = 0;

        // --------------------------------------------------------------------
        // Display
        // --------------------------------------------------------------------
        virtual f64 GetDisplayRefreshRate() const = 0;

        // --------------------------------------------------------------------
        // Factory – creates the platform-specific implementation.
        // --------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<IOS> Create();
    };

} // namespace engine::platform