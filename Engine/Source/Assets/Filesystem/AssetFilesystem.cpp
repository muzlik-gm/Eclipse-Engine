// ============================================================================
// File: Engine/Source/Assets/Filesystem/AssetFilesystem.cpp
// ============================================================================
#include "Engine/Assets/Filesystem/AssetFilesystem.h"
#include "Engine/Core/Log.h"

#include <filesystem>
#include <fstream>

namespace engine::assets {

    namespace fs = std::filesystem;

    using engine::core::u64;

    void AssetFilesystem::Mount(const std::string& scheme, const std::string& rootPath)
    {
        m_Roots[scheme] = rootPath;
    }

    void AssetFilesystem::Unmount(const std::string& scheme)
    {
        m_Roots.erase(scheme);
    }

    std::string AssetFilesystem::Resolve(const AssetPath& path) const
    {
        auto scheme = path.GetScheme();
        auto relPath = path.GetRelativePath();

        auto it = m_Roots.find(scheme);
        if (it == m_Roots.end())
            return "";

        fs::path result = fs::path(it->second) / relPath;
        return result.lexically_normal().string();
    }

    bool AssetFilesystem::Exists(const AssetPath& path) const
    {
        auto real = Resolve(path);
        if (real.empty())
            return false;
        return fs::exists(real);
    }

    u64 AssetFilesystem::GetFileSize(const AssetPath& path) const
    {
        auto real = Resolve(path);
        if (real.empty())
            return 0;
        std::error_code ec;
        auto size = fs::file_size(real, ec);
        return ec ? 0 : static_cast<u64>(size);
    }

    u64 AssetFilesystem::GetLastModified(const AssetPath& path) const
    {
        auto real = Resolve(path);
        if (real.empty())
            return 0;
        std::error_code ec;
        auto ftime = fs::last_write_time(real, ec);
        if (ec)
            return 0;
        return static_cast<u64>(std::chrono::duration_cast<std::chrono::seconds>(
            ftime.time_since_epoch()).count());
    }

    std::vector<AssetPath> AssetFilesystem::ScanDirectory(
        const AssetPath& directoryPath, const std::string& extension) const
    {
        std::vector<AssetPath> result;
        auto real = Resolve(directoryPath);
        if (real.empty() || !fs::exists(real))
            return result;

        for (const auto& entry : fs::directory_iterator(real))
        {
            if (!entry.is_regular_file())
                continue;

            auto ext = entry.path().extension().string();
            if (!extension.empty() && ext != extension)
                continue;

            result.push_back(AssetPath(directoryPath.GetScheme(),
                (fs::path(directoryPath.GetRelativePath()) / entry.path().filename()).string()));
        }

        return result;
    }

    std::vector<AssetPath> AssetFilesystem::ScanDirectoryRecursive(
        const AssetPath& directoryPath, const std::string& extension) const
    {
        std::vector<AssetPath> result;
        auto real = Resolve(directoryPath);
        if (real.empty() || !fs::exists(real))
            return result;

        for (const auto& entry : fs::recursive_directory_iterator(real))
        {
            if (!entry.is_regular_file())
                continue;

            auto ext = entry.path().extension().string();
            if (!extension.empty() && ext != extension)
                continue;

            auto rel = fs::relative(entry.path(), real);
            result.push_back(AssetPath(directoryPath.GetScheme(), rel.string()));
        }

        return result;
    }

    AssetPath AssetFilesystem::MakeVirtualPath(const std::string& realPath) const
    {
        for (const auto& [scheme, root] : m_Roots)
        {
            auto rootNorm = fs::path(root).lexically_normal();
            auto realNorm = fs::path(realPath).lexically_normal();

            auto rel = fs::relative(realNorm, rootNorm);
            if (!rel.empty() && rel.string() != ".")
            {
                return AssetPath(scheme, rel.string());
            }
        }
        return AssetPath{};
    }

    std::string AssetFilesystem::GetRoot(const std::string& scheme) const
    {
        auto it = m_Roots.find(scheme);
        return (it != m_Roots.end()) ? it->second : "";
    }

    std::vector<std::string> AssetFilesystem::GetSchemes() const
    {
        std::vector<std::string> result;
        result.reserve(m_Roots.size());
        for (const auto& [scheme, _] : m_Roots)
            result.push_back(scheme);
        return result;
    }

    bool AssetFilesystem::CreateDirectory(const AssetPath& path) const
    {
        auto real = Resolve(path);
        if (real.empty())
            return false;
        std::error_code ec;
        fs::create_directories(real, ec);
        return !ec;
    }

    bool AssetFilesystem::DeleteFile(const AssetPath& path) const
    {
        auto real = Resolve(path);
        if (real.empty())
            return false;
        std::error_code ec;
        return fs::remove(real, ec);
    }

    bool AssetFilesystem::MoveFile(const AssetPath& oldPath, const AssetPath& newPath) const
    {
        auto oldReal = Resolve(oldPath);
        auto newReal = Resolve(newPath);
        if (oldReal.empty() || newReal.empty())
            return false;
        std::error_code ec;
        fs::rename(oldReal, newReal, ec);
        return !ec;
    }

    bool AssetFilesystem::CopyFile(const AssetPath& srcPath, const AssetPath& dstPath) const
    {
        auto srcReal = Resolve(srcPath);
        auto dstReal = Resolve(dstPath);
        if (srcReal.empty() || dstReal.empty())
            return false;
        std::error_code ec;
        fs::copy_file(srcReal, dstReal, fs::copy_options::overwrite_existing, ec);
        return !ec;
    }

} // namespace engine::assets
