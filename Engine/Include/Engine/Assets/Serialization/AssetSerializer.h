// ============================================================================
// File: Engine/Include/Engine/Assets/Serialization/AssetSerializer.h
// Generic serialization framework for assets.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/Metadata/AssetMetadata.h"

#include <memory>
#include <string>
#include <vector>

namespace engine::assets {

    class IAsset;

    // ========================================================================
    // SerializationFormat — format for serialized asset data.
    // ========================================================================

    enum class SerializationFormat : core::u32
    {
        Binary = 0,
        JSON   = 1,
        YAML   = 2
    };

    // ========================================================================
    // IAssetSerializer — interface for asset serializers.
    // ========================================================================

    /// @brief Interface for serializing/deserializing assets.  Each
    ///        format (Binary, JSON, YAML) has a concrete implementation.
    class IAssetSerializer
    {
    public:
        virtual ~IAssetSerializer() = default;

        /// @brief Returns the serialization format.
        [[nodiscard]] virtual SerializationFormat GetFormat() const noexcept = 0;

        /// @brief Serializes @p asset to a byte string.
        [[nodiscard]] virtual std::string Serialize(const IAsset& asset) const = 0;

        /// @brief Serializes @p metadata to a byte string.
        [[nodiscard]] virtual std::string SerializeMetadata(const AssetMetadata& metadata) const = 0;

        /// @brief Deserializes @p data into metadata.
        [[nodiscard]] virtual AssetMetadata DeserializeMetadata(const std::string& data) const = 0;

        /// @brief Writes @p data to @p filePath.
        bool WriteToFile(const std::string& filePath, const std::string& data) const;

        /// @brief Reads the contents of @p filePath.
        [[nodiscard]] std::string ReadFromFile(const std::string& filePath) const;
    };

    // ========================================================================
    // BinarySerializer — binary format serializer.
    // ========================================================================

    class BinarySerializer final : public IAssetSerializer
    {
    public:
        [[nodiscard]] SerializationFormat GetFormat() const noexcept override
        { return SerializationFormat::Binary; }

        [[nodiscard]] std::string Serialize(const IAsset& asset) const override;
        [[nodiscard]] std::string SerializeMetadata(const AssetMetadata& metadata) const override;
        [[nodiscard]] AssetMetadata DeserializeMetadata(const std::string& data) const override;
    };

    // ========================================================================
    // JsonSerializer — JSON format serializer.
    // ========================================================================

    class JsonSerializer final : public IAssetSerializer
    {
    public:
        [[nodiscard]] SerializationFormat GetFormat() const noexcept override
        { return SerializationFormat::JSON; }

        [[nodiscard]] std::string Serialize(const IAsset& asset) const override;
        [[nodiscard]] std::string SerializeMetadata(const AssetMetadata& metadata) const override;
        [[nodiscard]] AssetMetadata DeserializeMetadata(const std::string& data) const override;
    };

    // ========================================================================
    // YamlSerializer — YAML format serializer.
    // ========================================================================

    class YamlSerializer final : public IAssetSerializer
    {
    public:
        [[nodiscard]] SerializationFormat GetFormat() const noexcept override
        { return SerializationFormat::YAML; }

        [[nodiscard]] std::string Serialize(const IAsset& asset) const override;
        [[nodiscard]] std::string SerializeMetadata(const AssetMetadata& metadata) const override;
        [[nodiscard]] AssetMetadata DeserializeMetadata(const std::string& data) const override;
    };

} // namespace engine::assets
