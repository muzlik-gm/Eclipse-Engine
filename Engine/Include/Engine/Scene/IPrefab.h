// ============================================================================
// File: Engine/Include/Engine/Scene/IPrefab.h
// Abstract interface for prefabs — reusable entity templates.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/UUID.h"
#include "Engine/ECS/Entity.h"

#include <memory>
#include <string>

namespace engine::scene {

    class Scene;

    // ========================================================================
    // IPrefab — abstract interface for a prefab asset.
    // ========================================================================

    /// @brief A prefab is a reusable entity template.  It stores a snapshot
    ///        of an entity and its component data that can be instantiated
    ///        multiple times into a Scene.
    ///
    /// Concrete implementations (JSON prefab, binary prefab, etc.) are
    /// provided by future phases.  This interface defines the contract
    /// that all prefab backends must satisfy.
    class IPrefab
    {
    public:
        virtual ~IPrefab() = default;

        /// @brief Returns the prefab's UUID.
        [[nodiscard]] virtual const core::UUID& GetUUID() const noexcept = 0;

        /// @brief Returns the prefab's human-readable name.
        [[nodiscard]] virtual const std::string& GetName() const noexcept = 0;

        /// @brief Instantiates the prefab into @p scene as a new entity.
        /// @return The newly created entity handle.
        virtual ecs::Entity Instantiate(Scene& scene) const = 0;

        /// @brief Instantiates the prefab into @p scene as a child of
        ///        @p parent.
        virtual ecs::Entity InstantiateAsChild(Scene& scene, ecs::Entity parent) const = 0;

        /// @brief Returns the number of entities in this prefab (including
        ///        the root and all descendants).
        [[nodiscard]] virtual core::usize GetEntityCount() const noexcept = 0;

        /// @brief Returns true if the prefab contains any entities.
        [[nodiscard]] virtual bool IsValid() const noexcept = 0;
    };

    // ========================================================================
    // PrefabData — in-memory prefab storage.
    // ========================================================================

    /// @brief In-memory representation of a prefab.  Stores a serialized
    ///        snapshot of an entity hierarchy that can be instantiated
    ///        into any Scene.
    struct PrefabData
    {
        core::UUID    UUID;
        std::string   Name;

        /// Serialized entity data (format is implementation-defined).
        /// The concrete Prefab implementation interprets this blob.
        std::string   SerializedData;

        /// Number of entities in this prefab.
        core::usize   EntityCount{0};
    };

    // ========================================================================
    // IPrefabLoader — abstract interface for loading prefabs from storage.
    // ========================================================================

    class IPrefabLoader
    {
    public:
        virtual ~IPrefabLoader() = default;

        /// @brief Loads a prefab from @p filePath.
        [[nodiscard]] virtual std::unique_ptr<IPrefab> Load(const std::string& filePath) = 0;

        /// @brief Returns true if this loader can handle the given extension.
        [[nodiscard]] virtual bool SupportsExtension(const std::string& extension) const = 0;
    };

    // ========================================================================
    // IPrefabSaver — abstract interface for saving prefabs to storage.
    // ========================================================================

    class IPrefabSaver
    {
    public:
        virtual ~IPrefabSaver() = default;

        /// @brief Saves @p prefab to @p filePath.
        virtual bool Save(const IPrefab& prefab, const std::string& filePath) = 0;

        /// @brief Creates a prefab from an entity in @p scene.
        [[nodiscard]] virtual std::unique_ptr<IPrefab> CreateFromEntity(
            Scene& scene, ecs::Entity entity, const std::string& name) = 0;

        /// @brief Returns the default file extension for this saver.
        [[nodiscard]] virtual std::string GetDefaultExtension() const = 0;
    };

} // namespace engine::scene
