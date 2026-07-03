// ============================================================================
// File: Engine/Source/Runtime/SubsystemManager.cpp
// Implementation of the subsystem registration, dependency-ordered init,
// and per-frame update dispatching.
// ============================================================================

#include "Engine/Runtime/SubsystemManager.h"
#include "Engine/Core/Log.h"

#include <algorithm>
#include <queue>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace engine::runtime
{

    // ========================================================================
    // Registration
    // ========================================================================

    void SubsystemManager::Register(std::unique_ptr<ISubsystem> subsystem)
    {
        if (!subsystem)
        {
            ENGINE_LOG_WARN("SubsystemManager — attempted to register a null subsystem");
            return;
        }

        const std::string name = std::string(subsystem->GetName());

        // Shut down and replace if already registered.
        auto it = m_subsystems.find(name);
        if (it != m_subsystems.end())
        {
            if (m_initialized[name] && it->second)
            {
                ENGINE_LOG_INFO("SubsystemManager — shutting down existing subsystem '{}' before replacement", name);
                try
                {
                    it->second->Shutdown();
                }
                catch (const std::exception& e)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — shutdown of '{}' threw: {}", name, e.what());
                }
                catch (...)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — shutdown of '{}' threw an unknown exception", name);
                }
                m_initialized[name] = false;
            }
            ENGINE_LOG_DEBUG("SubsystemManager — replacing subsystem '{}'", name);
        }
        else
        {
            ENGINE_LOG_DEBUG("SubsystemManager — registered subsystem '{}'", name);
        }

        m_subsystems[name] = std::move(subsystem);
        m_initialized[name] = false;

        // Invalidate cached init order.
        m_initOrder.clear();
    }

    bool SubsystemManager::Unregister(std::string_view name)
    {
        const std::string key(name);

        auto it = m_subsystems.find(key);
        if (it == m_subsystems.end())
        {
            return false;
        }

        if (m_initialized[key] && it->second)
        {
            ENGINE_LOG_INFO("SubsystemManager — shutting down '{}' before unregister", key);
            try
            {
                it->second->Shutdown();
            }
            catch (const std::exception& e)
            {
                ENGINE_LOG_ERROR("SubsystemManager — shutdown threw: {}", e.what());
            }
            catch (...)
            {
                ENGINE_LOG_ERROR("SubsystemManager — shutdown threw an unknown exception");
            }
        }

        m_subsystems.erase(it);
        m_initialized.erase(key);
        m_initOrder.clear();

        ENGINE_LOG_DEBUG("SubsystemManager — unregistered subsystem '{}'", key);
        return true;
    }

    // ========================================================================
    // Lookup
    // ========================================================================

    ISubsystem* SubsystemManager::GetRaw(std::string_view name) const noexcept
    {
        auto it = m_subsystems.find(std::string(name));
        if (it != m_subsystems.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    bool SubsystemManager::Has(std::string_view name) const noexcept
    {
        return m_subsystems.find(std::string(name)) != m_subsystems.end();
    }

    usize SubsystemManager::Count() const noexcept
    {
        return m_subsystems.size();
    }

    std::vector<std::string> SubsystemManager::GetNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_subsystems.size());
        for (const auto& [name, _] : m_subsystems)
        {
            names.push_back(name);
        }
        return names;
    }

    bool SubsystemManager::IsInitialized(std::string_view name) const noexcept
    {
        auto it = m_initialized.find(std::string(name));
        return it != m_initialized.end() && it->second;
    }

    // ========================================================================
    // Lifecycle
    // ========================================================================

    bool SubsystemManager::ValidateDependencies(std::string& outError) const
    {
        for (const auto& [name, subsystem] : m_subsystems)
        {
            const auto deps = subsystem->GetDependencies();
            for (const auto& dep : deps)
            {
                if (m_subsystems.find(dep) == m_subsystems.end())
                {
                    outError = "Subsystem '" + name +
                               "' depends on unregistered subsystem '" +
                               dep + "'";
                    return false;
                }
            }
        }
        return true;
    }

    bool SubsystemManager::ComputeInitOrder(std::vector<std::string>& outOrder) const
    {
        outOrder.clear();

        // Kahn's algorithm for topological sort.
        // Build in-degree map.
        std::unordered_map<std::string, usize> inDegree;
        std::unordered_map<std::string, std::vector<std::string>> dependents;

        for (const auto& [name, _] : m_subsystems)
        {
            inDegree[name] = 0;
        }

        for (const auto& [name, subsystem] : m_subsystems)
        {
            for (const auto& dep : subsystem->GetDependencies())
            {
                // dep must come before name.
                ++inDegree[name];
                dependents[dep].push_back(name);
            }
        }

        // Queue all nodes with in-degree 0.
        // Use a sorted container for deterministic output.
        std::queue<std::string> queue;
        for (const auto& [name, degree] : inDegree)
        {
            if (degree == 0)
            {
                queue.push(name);
            }
        }

        usize processed = 0;
        while (!queue.empty())
        {
            const std::string current = queue.front();
            queue.pop();
            outOrder.push_back(current);
            ++processed;

            for (const auto& dependent : dependents[current])
            {
                --inDegree[dependent];
                if (inDegree[dependent] == 0)
                {
                    queue.push(dependent);
                }
            }
        }

        // If not all nodes processed, there is a cycle.
        return processed == m_subsystems.size();
    }

    bool SubsystemManager::InitializeAll()
    {
        // Validate dependencies.
        std::string depError;
        if (!ValidateDependencies(depError))
        {
            ENGINE_LOG_ERROR("SubsystemManager — dependency validation failed: {}", depError);
            return false;
        }

        // Compute init order.
        std::vector<std::string> order;
        if (!ComputeInitOrder(order))
        {
            ENGINE_LOG_ERROR("SubsystemManager — dependency cycle detected among subsystems");
            return false;
        }

        m_initOrder = order;

        // Initialize in order.
        for (const auto& name : order)
        {
            auto it = m_subsystems.find(name);
            if (it == m_subsystems.end() || !it->second)
            {
                ENGINE_LOG_WARN("SubsystemManager — subsystem '{}' not found during init", name);
                continue;
            }

            if (m_initialized[name])
            {
                ENGINE_LOG_DEBUG("SubsystemManager — subsystem '{}' already initialized, skipping", name);
                continue;
            }

            ENGINE_LOG_INFO("SubsystemManager — initializing subsystem '{}'", name);

            try
            {
                const bool success = it->second->Initialize();
                if (!success)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' initialization failed", name);
                    return false;
                }
            }
            catch (const std::exception& e)
            {
                ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' initialization threw: {}", name, e.what());
                return false;
            }
            catch (...)
            {
                ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' initialization threw an unknown exception", name);
                return false;
            }

            m_initialized[name] = true;
            ENGINE_LOG_INFO("SubsystemManager — subsystem '{}' initialized successfully", name);
        }

        ENGINE_LOG_INFO("SubsystemManager — all {} subsystem(s) initialized", m_subsystems.size());
        return true;
    }

    void SubsystemManager::ShutdownAll()
    {
        // Shut down in reverse init order.
        if (m_initOrder.empty())
        {
            // Compute order in case InitializeAll was never called.
            (void)ComputeInitOrder(m_initOrder);
        }

        for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
        {
            const auto& name = *it;
            auto subIt = m_subsystems.find(name);
            if (subIt == m_subsystems.end() || !subIt->second)
            {
                continue;
            }

            if (!m_initialized[name])
            {
                continue;
            }

            ENGINE_LOG_INFO("SubsystemManager — shutting down subsystem '{}'", name);

            try
            {
                subIt->second->Shutdown();
            }
            catch (const std::exception& e)
            {
                ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' shutdown threw: {}", name, e.what());
            }
            catch (...)
            {
                ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' shutdown threw an unknown exception", name);
            }

            m_initialized[name] = false;
        }

        ENGINE_LOG_INFO("SubsystemManager — all subsystems shut down");
    }

    // ========================================================================
    // Per-frame updates
    // ========================================================================

    void SubsystemManager::UpdateAll(f64 deltaTime)
    {
        for (const auto& name : m_initOrder)
        {
            auto it = m_subsystems.find(name);
            if (it != m_subsystems.end() && it->second && m_initialized[name])
            {
                try
                {
                    it->second->Update(deltaTime);
                }
                catch (const std::exception& e)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' Update threw: {}", name, e.what());
                }
                catch (...)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' Update threw an unknown exception", name);
                }
            }
        }
    }

    void SubsystemManager::FixedUpdateAll(f64 fixedDeltaTime)
    {
        for (const auto& name : m_initOrder)
        {
            auto it = m_subsystems.find(name);
            if (it != m_subsystems.end() && it->second && m_initialized[name])
            {
                try
                {
                    it->second->FixedUpdate(fixedDeltaTime);
                }
                catch (const std::exception& e)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' FixedUpdate threw: {}", name, e.what());
                }
                catch (...)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' FixedUpdate threw an unknown exception", name);
                }
            }
        }
    }

    void SubsystemManager::LateUpdateAll(f64 deltaTime)
    {
        for (const auto& name : m_initOrder)
        {
            auto it = m_subsystems.find(name);
            if (it != m_subsystems.end() && it->second && m_initialized[name])
            {
                try
                {
                    it->second->LateUpdate(deltaTime);
                }
                catch (const std::exception& e)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' LateUpdate threw: {}", name, e.what());
                }
                catch (...)
                {
                    ENGINE_LOG_ERROR("SubsystemManager — subsystem '{}' LateUpdate threw an unknown exception", name);
                }
            }
        }
    }

} // namespace engine::runtime