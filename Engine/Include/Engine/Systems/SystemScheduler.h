// ============================================================================
// File: Engine/Include/Engine/Systems/SystemScheduler.h
// Manages system execution order via groups, priorities, and dependencies.
// ============================================================================
#pragma once

#include "Engine/Systems/ISystem.h"
#include "Engine/Core/Types.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine::systems {

    using engine::core::f64;
    using engine::core::i32;
    using engine::core::usize;

    /// @brief Bookkeeping for each registered system.
    struct SystemDescriptor
    {
        std::unique_ptr<ISystem>  System;
        std::string              Group{"Default"};
        i32                      Priority{0};
        std::unordered_set<std::string> Dependencies;
    };

    /// @brief Manages system registration, ordering, and execution.
    class SystemScheduler
    {
    public:
        SystemScheduler()  = default;
        ~SystemScheduler() = default;

        template <typename T, typename... Args>
        T& Add(ecs::Registry& registry, const std::string& group = "Default",
               i32 priority = 0, Args&&... args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            system->OnAttach(registry);

            SystemDescriptor desc;
            desc.System   = std::move(system);
            desc.Group    = group;
            desc.Priority = priority;

            T& ref = static_cast<T&>(*desc.System);
            m_Systems.push_back(std::move(desc));
            m_SortDirty = true;
            return ref;
        }

        void Clear();

        void Update(f64 deltaTime);
        void FixedUpdate(f64 fixedDeltaTime);
        void LateUpdate(f64 deltaTime);

        [[nodiscard]] usize SystemCount() const { return m_Systems.size(); }
        [[nodiscard]] ISystem* GetSystem(usize index);
        [[nodiscard]] ISystem* GetSystemByName(std::string_view name);

    private:
        void SortSystems();

        std::vector<SystemDescriptor> m_Systems;
        bool m_SortDirty{true};
    };

} // namespace engine::systems
