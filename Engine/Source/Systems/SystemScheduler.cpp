// ============================================================================
// File: Engine/Source/Systems/SystemScheduler.cpp
// ============================================================================
#include "Engine/Systems/SystemScheduler.h"
#include <algorithm>

namespace engine::systems {

    void SystemScheduler::Clear()
    {
        for (auto& desc : m_Systems)
        {
            if (desc.System)
                desc.System->OnDetach();
        }
        m_Systems.clear();
        m_SortDirty = true;
    }

    void SystemScheduler::Update(f64 deltaTime)
    {
        SortSystems();
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->IsEnabled())
                desc.System->Update(deltaTime);
        }
    }

    void SystemScheduler::FixedUpdate(f64 fixedDeltaTime)
    {
        SortSystems();
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->IsEnabled())
                desc.System->FixedUpdate(fixedDeltaTime);
        }
    }

    void SystemScheduler::LateUpdate(f64 deltaTime)
    {
        SortSystems();
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->IsEnabled())
                desc.System->LateUpdate(deltaTime);
        }
    }

    ISystem* SystemScheduler::GetSystem(usize index)
    {
        SortSystems();
        return (index < m_Systems.size()) ? m_Systems[index].System.get() : nullptr;
    }

    ISystem* SystemScheduler::GetSystemByName(std::string_view name)
    {
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->GetName() == name)
                return desc.System.get();
        }
        return nullptr;
    }

    void SystemScheduler::SortSystems()
    {
        if (!m_SortDirty)
            return;

        std::stable_sort(m_Systems.begin(), m_Systems.end(),
            [](const SystemDescriptor& a, const SystemDescriptor& b)
            {
                if (a.Group != b.Group)
                    return a.Group < b.Group;
                return a.Priority < b.Priority;
            });

        m_SortDirty = false;
    }

} // namespace engine::systems
