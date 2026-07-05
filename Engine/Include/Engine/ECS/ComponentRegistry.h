// ============================================================================
// File: Engine/Include/Engine/ECS/ComponentRegistry.h
// Runtime type registry for components.
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

    using engine::core::usize;

    /// @brief Stores runtime metadata about a registered component type.
    struct ComponentMeta
    {
        std::string     Name;
        std::type_index TypeIndex;
        usize           Size;
        std::function<void(Registry&, Entity)> RemoveFn;
    };

    /// @brief Registry that stores metadata for every registered component type.
    class ComponentRegistry
    {
    public:
        template <typename T>
        void Register(const std::string& name)
        {
            ComponentMeta meta;
            meta.Name      = name;
            meta.TypeIndex = std::type_index(typeid(T));
            meta.Size      = sizeof(T);
            meta.RemoveFn   = [](Registry& reg, Entity ent) {
                if (reg.HasComponent<T>(ent))
                    reg.RemoveComponent<T>(ent);
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

        [[nodiscard]] usize Count() const { return m_Components.size(); }

        [[nodiscard]] const std::vector<std::string>& GetNames() const
        {
            return m_OrderedNames;
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
