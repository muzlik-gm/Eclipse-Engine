#include "Engine/Platform/FileDialog.h"
#include "Engine/Core/Log.h"

#include <optional>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>

namespace engine::platform
{

    class Win32FileDialog final : public IFileDialog
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
            if (type == FileDialogType::OpenFolder)
                return OpenFolderDialog();
            else if (type == FileDialogType::OpenFile)
                return OpenFileDialog();
            else if (type == FileDialogType::SaveFile)
                return SaveFileDialog();
            return std::nullopt;
        }

        [[nodiscard]] std::vector<std::string> OpenMultiple() override
        {
            ENGINE_LOG_WARN("Win32FileDialog — OpenMultiple not yet implemented");
            return {};
        }

    private:
        std::optional<std::string> OpenFolderDialog()
        {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
            if (FAILED(hr))
            {
                ENGINE_LOG_WARN("Win32FileDialog — CoInitializeEx failed (0x{:X})", hr);
                return std::nullopt;
            }

            IFileOpenDialog* pDialog = nullptr;
            hr = CoCreateInstance(
                CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&pDialog));

            if (FAILED(hr))
            {
                ENGINE_LOG_WARN("Win32FileDialog — CoCreateInstance(FileOpenDialog) failed (0x{:X})", hr);
                CoUninitialize();
                return std::nullopt;
            }

            // Set folder picker options.
            DWORD options = 0;
            pDialog->GetOptions(&options);
            pDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

            if (!m_Title.empty())
            {
                std::wstring wTitle(m_Title.begin(), m_Title.end());
                pDialog->SetTitle(wTitle.c_str());
            }

            std::optional<std::string> result = std::nullopt;

            hr = pDialog->Show(nullptr);
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem = nullptr;
                hr = pDialog->GetResult(&pItem);
                if (SUCCEEDED(hr) && pItem)
                {
                    PWSTR pszPath = nullptr;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                    if (SUCCEEDED(hr) && pszPath)
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, nullptr, 0, nullptr, nullptr);
                        if (len > 0)
                        {
                            std::string path(static_cast<size_t>(len) - 1, '\0');
                            WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, path.data(), len, nullptr, nullptr);
                            result = path;
                        }
                        CoTaskMemFree(pszPath);
                    }
                    pItem->Release();
                }
            }
            else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                // User cancelled — that's fine, no log needed.
            }
            else
            {
                ENGINE_LOG_WARN("Win32FileDialog — Show failed (0x{:X})", hr);
            }

            pDialog->Release();
            CoUninitialize();
            return result;
        }

        std::optional<std::string> OpenFileDialog()
        {
            ENGINE_LOG_WARN("Win32FileDialog — OpenFile not yet implemented");
            (void)m_Filters;
            (void)m_DefaultExtension;
            return std::nullopt;
        }

        std::optional<std::string> SaveFileDialog()
        {
            ENGINE_LOG_WARN("Win32FileDialog — SaveFile not yet implemented");
            return std::nullopt;
        }

        std::string                m_Title;
        std::string                m_DefaultExtension;
        std::vector<FileDialogFilter> m_Filters;
    };

    std::unique_ptr<IFileDialog> IFileDialog::Create()
    {
        return std::make_unique<Win32FileDialog>();
    }

} // namespace engine::platform
