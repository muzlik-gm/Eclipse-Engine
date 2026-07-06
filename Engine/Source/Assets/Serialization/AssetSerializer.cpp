// ============================================================================
// File: Engine/Source/Assets/Serialization/AssetSerializer.cpp
// ============================================================================
#include "Engine/Assets/Serialization/AssetSerializer.h"
#include "Engine/Assets/Core/IAsset.h"
#include "Engine/Core/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

namespace engine::assets {

    using json = nlohmann::json;

    // ========================================================================
    // IAssetSerializer base — file I/O helpers
    // ========================================================================

    bool IAssetSerializer::WriteToFile(const std::string& filePath, const std::string& data) const
    {
        std::ofstream ofs(filePath, std::ios::binary);
        if (!ofs.is_open())
        {
            ENGINE_LOG_ERROR("AssetSerializer — cannot open '{}' for writing", filePath);
            return false;
        }
        ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
        return true;
    }

    std::string IAssetSerializer::ReadFromFile(const std::string& filePath) const
    {
        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs.is_open())
        {
            ENGINE_LOG_ERROR("AssetSerializer — cannot open '{}' for reading", filePath);
            return "";
        }
        std::ostringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }

    // ========================================================================
    // Helpers for JSON metadata serialization
    // ========================================================================

    static json MetadataToJson(const AssetMetadata& meta)
    {
        json j;
        j["uuid"]              = meta.UUID.ToString();
        j["name"]              = meta.Name;
        j["path"]              = meta.Path.Path;
        j["type"]              = static_cast<u32>(meta.Type);
        j["custom_type_name"]  = meta.CustomTypeName;
        j["source_file"]       = meta.SourceFilePath;
        j["imported_file"]     = meta.ImportedFilePath;
        j["version"]           = meta.Version;
        j["importer_version"]  = meta.ImporterVersion;
        j["source_size"]       = meta.SourceFileSize;
        j["source_modified"]   = meta.SourceLastModified;
        j["included_in_build"] = meta.IsIncludedInBuild;
        j["is_streamed"]       = meta.IsStreamed;
        j["streaming_priority"] = meta.StreamingPriority;

        // Dependencies
        json deps = json::array();
        for (const auto& d : meta.Dependencies)
            deps.push_back(d.ToString());
        j["dependencies"] = deps;

        // Referenced by
        json refs = json::array();
        for (const auto& r : meta.ReferencedBy)
            refs.push_back(r.ToString());
        j["referenced_by"] = refs;

        // Tags
        json tags = json::array();
        for (const auto& [k, v] : meta.Tags)
        {
            json tag;
            tag["key"] = k;
            tag["value"] = v;
            tags.push_back(tag);
        }
        j["tags"] = tags;

        return j;
    }

    static AssetMetadata JsonToMetadata(const json& j)
    {
        AssetMetadata meta;

        if (j.contains("uuid"))
            meta.UUID = AssetUUID::FromString(j["uuid"].get<std::string>());
        if (j.contains("name"))
            meta.Name = j["name"].get<std::string>();
        if (j.contains("path"))
            meta.Path = AssetPath(j["path"].get<std::string>());
        if (j.contains("type"))
            meta.Type = static_cast<AssetType>(j["type"].get<u32>());
        if (j.contains("custom_type_name"))
            meta.CustomTypeName = j["custom_type_name"].get<std::string>();
        if (j.contains("source_file"))
            meta.SourceFilePath = j["source_file"].get<std::string>();
        if (j.contains("imported_file"))
            meta.ImportedFilePath = j["imported_file"].get<std::string>();
        if (j.contains("version"))
            meta.Version = j["version"].get<u32>();
        if (j.contains("importer_version"))
            meta.ImporterVersion = j["importer_version"].get<u32>();
        if (j.contains("source_size"))
            meta.SourceFileSize = j["source_size"].get<u64>();
        if (j.contains("source_modified"))
            meta.SourceLastModified = j["source_modified"].get<u64>();
        if (j.contains("included_in_build"))
            meta.IsIncludedInBuild = j["included_in_build"].get<bool>();
        if (j.contains("is_streamed"))
            meta.IsStreamed = j["is_streamed"].get<bool>();
        if (j.contains("streaming_priority"))
            meta.StreamingPriority = j["streaming_priority"].get<u32>();

        if (j.contains("dependencies"))
        {
            for (const auto& d : j["dependencies"])
                meta.Dependencies.push_back(AssetUUID::FromString(d.get<std::string>()));
        }

        if (j.contains("referenced_by"))
        {
            for (const auto& r : j["referenced_by"])
                meta.ReferencedBy.push_back(AssetUUID::FromString(r.get<std::string>()));
        }

        if (j.contains("tags"))
        {
            for (const auto& t : j["tags"])
            {
                meta.Tags.emplace_back(
                    t["key"].get<std::string>(),
                    t["value"].get<std::string>()
                );
            }
        }

        return meta;
    }

    // ========================================================================
    // BinarySerializer
    // ========================================================================

    std::string BinarySerializer::Serialize(const IAsset& asset) const
    {
        // For binary, we write a simple header + metadata JSON + raw data.
        // In production this would use a proper binary format.
        json j;
        j["metadata"] = MetadataToJson(asset.GetMetadata());
        j["type"] = static_cast<u32>(asset.GetType());
        j["size"] = static_cast<u64>(asset.GetSize());
        return j.dump();
    }

    std::string BinarySerializer::SerializeMetadata(const AssetMetadata& metadata) const
    {
        return MetadataToJson(metadata).dump();
    }

    AssetMetadata BinarySerializer::DeserializeMetadata(const std::string& data) const
    {
        try
        {
            auto j = json::parse(data);
            return JsonToMetadata(j);
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("BinarySerializer — parse error: {}", e.what());
            return AssetMetadata{};
        }
    }

    // ========================================================================
    // JsonSerializer
    // ========================================================================

    std::string JsonSerializer::Serialize(const IAsset& asset) const
    {
        json j;
        j["metadata"] = MetadataToJson(asset.GetMetadata());
        j["type"] = static_cast<u32>(asset.GetType());
        j["size"] = static_cast<u64>(asset.GetSize());
        j["state"] = static_cast<u32>(asset.GetState());
        return j.dump(4);
    }

    std::string JsonSerializer::SerializeMetadata(const AssetMetadata& metadata) const
    {
        return MetadataToJson(metadata).dump(4);
    }

    AssetMetadata JsonSerializer::DeserializeMetadata(const std::string& data) const
    {
        try
        {
            auto j = json::parse(data);
            return JsonToMetadata(j);
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("JsonSerializer — parse error: {}", e.what());
            return AssetMetadata{};
        }
    }

    // ========================================================================
    // YamlSerializer — uses JSON as a fallback (YAML emitter not linked).
    // ========================================================================

    std::string YamlSerializer::Serialize(const IAsset& asset) const
    {
        // Use JSON serialization as a fallback.  A proper YAML emitter
        // would be linked in a future update.
        json j;
        j["metadata"] = MetadataToJson(asset.GetMetadata());
        j["type"] = static_cast<u32>(asset.GetType());
        j["size"] = static_cast<u64>(asset.GetSize());
        return j.dump(4);
    }

    std::string YamlSerializer::SerializeMetadata(const AssetMetadata& metadata) const
    {
        return MetadataToJson(metadata).dump(4);
    }

    AssetMetadata YamlSerializer::DeserializeMetadata(const std::string& data) const
    {
        try
        {
            auto j = json::parse(data);
            return JsonToMetadata(j);
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("YamlSerializer — parse error: {}", e.what());
            return AssetMetadata{};
        }
    }

} // namespace engine::assets
