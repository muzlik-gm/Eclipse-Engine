// ============================================================================
// File: Engine/Include/Engine/ECS/ComponentRegistry.h
// Runtime type registry for components with versioning support.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"

#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace engine::ecs {

    using engine::core::u32;
    using engine::core::usize;

    /// @brief Stores runtime metadata about a registered component type.
    struct ComponentMeta
    {
        std::string     Name;
        std::type_index TypeIndex;
        usize           Size;

        /// Schema version of the component.  Increment this when the
        /// component's data layout changes so the serialization system
        /// can perform migrations.
        u32             Version{1};

        /// Function that removes the component from an entity.
        std::function<void(Registry&, Entity)> RemoveFn;

        /// Function that returns true if the entity has this component.
        std::function<bool(Registry&, Entity)> HasFn;

        /// Optional function that creates a default-constructed instance
        /// of the component on the entity.  Used by the "Add Component"
        /// editor action when the component type is only known at runtime.
        std::function<void(Registry&, Entity)> CreateFn;

        /// Optional function that clones the component from one entity
        /// to another.  Used by prefab instantiation and duplication.
        std::function<void(Registry&, Entity src, Entity dst)> CloneFn;
    };

    /// @brief Registry that stores metadata for every registered component type.
    class ComponentRegistry
    {
    public:
        /// @brief Registers a component type with default version 1.
        template <typename T>
        void Register(const std::string& name)
        {
            RegisterWithVersion<T>(name, 1);
        }

        /// @brief Registers a component type with an explicit version.
        template <typename T>
        void RegisterWithVersion(const std::string& name, u32 version)
        {
            ComponentMeta meta;
            meta.Name      = name;
            meta.TypeIndex = std::type_index(typeid(T));
            meta.Size      = sizeof(T);
            meta.Version   = version;
            meta.RemoveFn  = [](Registry& reg, Entity ent) {
                if (reg.HasComponent<T>(ent))
                    reg.RemoveComponent<T>(ent);
            };
            meta.HasFn     = [](Registry& reg, Entity ent) {
                return reg.HasComponent<T>(ent);
            };
            meta.CreateFn  = [](Registry& reg, Entity ent) {
                if (!reg.HasComponent<T>(ent))
                    reg.AddComponent<T>(ent);
            };
            meta.CloneFn   = [](Registry& reg, Entity src, Entity dst) {
                if (reg.HasComponent<T>(src))
                    reg.AddComponent<T>(dst, reg.GetComponent<T>(src));
            };

            m_Components[std::type_index(typeid(T))] = std::move(meta);
            m_OrderedNames.push_back(name);
        }

        [[nodiscard]] const ComponentMeta* GetMeta(std::type_index idx) const
        {
            auto it = m_Components.find(idx);
            return (it != m_Components.end()) ? &it->second : nullptr;
        }

        [[nodiscard]] const ComponentMeta* GetMetaByName(const std::string& name) const
        {
            for (const auto& [idx, meta] : m_Components)
            {
                if (meta.Name == name)
                    return &meta;
            }
            return nullptr;
        }

        /// @brief Returns the version of a registered component type, or 0
        ///        if not registered.
        [[nodiscard]] u32 GetVersion(std::type_index idx) const
        {
            auto* meta = GetMeta(idx);
            return meta ? meta->Version : 0;
        }

        /// @brief Returns the version of a registered component type by name.
        [[nodiscard]] u32 GetVersionByName(const std::string& name) const
        {
            auto* meta = GetMetaByName(name);
            return meta ? meta->Version : 0;
        }

        [[nodiscard]] usize Count() const { return m_Components.size(); }

        [[nodiscard]] const std::vector<std::string>& GetNames() const
        {
            return m_OrderedNames;
        }

        /// @brief Returns all registered component metadata.
        [[nodiscard]] std::vector<const ComponentMeta*> GetAllMeta() const
        {
            std::vector<const ComponentMeta*> result;
            result.reserve(m_Components.size());
            for (const auto& [idx, meta] : m_Components)
                result.push_back(&meta);
            return result;
        }

        void Clear()
        {
            m_Components.clear();
            m_OrderedNames.clear();
        }

    private:
        std::unordered_map<std::type_index, ComponentMeta> m_Components;
        std::vector<std::string>                           m_OrderedNames;
    };

} // namespace engine::ecs
