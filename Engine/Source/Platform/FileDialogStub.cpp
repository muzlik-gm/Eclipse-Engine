// ============================================================================
// File: Engine/Source/Platform/FileDialogStub.cpp
// ============================================================================
//
// Stub implementation of IFileDialog. Provides a no-op backend for platforms
// that do not yet have a native file dialog integration.
//
// ============================================================================

#include "Engine/Platform/FileDialog.h"
#include "Engine/Core/Log.h"

#include <optional>
#include <string>
#include <vector>

namespace engine::platform
{

    // ========================================================================
    // StubFileDialog
    // ========================================================================

    class StubFileDialog final : public IFileDialog
    {
    public:
        void SetTitle(const std::string& title) override
        {
            m_Title = title;
        }

        void AddFilter(const FileDialogFilter& filter) override
        {
            m_Filters.push_back(filter);
        }

        void ClearFilters() override
        {
            m_Filters.clear();
        }

        void SetDefaultExtension(const std::string& extension) override
        {
            m_DefaultExtension = extension;
        }

        [[nodiscard]] std::optional<std::string> Open(FileDialogType type) override
        {
            ENGINE_LOG_WARN("FileDialog — native file dialog not implemented; returning nullopt");
            (void)type;
            return std::nullopt;
        }

        [[nodiscard]] std::vector<std::string> OpenMultiple() override
        {
            ENGINE_LOG_WARN("FileDialog — native file dialog not implemented; returning empty list");
            return {};
        }

    private:
        std::string                m_Title;
        std::string                m_DefaultExtension;
        std::vector<FileDialogFilter> m_Filters;
    };

    // ========================================================================
    // IFileDialog factory
    // ========================================================================

    std::unique_ptr<IFileDialog> IFileDialog::Create()
    {
        return std::make_unique<StubFileDialog>();
    }

} // namespace engine::platform