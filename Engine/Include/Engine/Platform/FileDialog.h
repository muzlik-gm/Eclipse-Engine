// ============================================================================
// File: Engine/Include/Engine/Platform/FileDialog.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace engine::platform {

    // ========================================================================
    // FileDialogType
    // ========================================================================
    enum class FileDialogType
    {
        OpenFile   = 0,
        SaveFile   = 1,
        OpenFolder = 2
    };

    // ========================================================================
    // FileDialogFilter
    // ========================================================================
    struct FileDialogFilter
    {
        std::string            Name;
        std::vector<std::string> Extensions;
    };

    // ========================================================================
    // IFileDialog – pure-virtual file dialog interface
    // ========================================================================
    class IFileDialog : private engine::core::NonCopyable
    {
    public:
        virtual ~IFileDialog() = default;

        /// Sets the title displayed in the file dialog window.
        virtual void SetTitle(const std::string& title) = 0;

        /// Adds a file type filter (e.g. Name="Images", Extensions={"png","jpg"}).
        virtual void AddFilter(const FileDialogFilter& filter) = 0;

        /// Removes all previously added filters.
        virtual void ClearFilters() = 0;

        /// Sets the default file extension used for save dialogs (e.g. "png").
        virtual void SetDefaultExtension(const std::string& extension) = 0;

        /// Opens a modal file dialog for a single file selection.
        /// Returns the selected file path, or std::nullopt if the user cancelled.
        [[nodiscard]] virtual std::optional<std::string> Open(FileDialogType type) = 0;

        /// Opens a modal file dialog allowing multiple file selection.
        /// Returns a vector of selected file paths, or an empty vector if cancelled.
        /// Only valid for FileDialogType::OpenFile; other types return empty.
        [[nodiscard]] virtual std::vector<std::string> OpenMultiple() = 0;

        // --------------------------------------------------------------------
        // Factory – creates the platform-specific implementation.
        // --------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<IFileDialog> Create();
    };

} // namespace engine::platform