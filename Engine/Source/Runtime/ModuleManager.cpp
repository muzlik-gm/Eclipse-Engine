// ============================================================================
// File: Engine/Source/Runtime/ModuleManager.cpp
// Implementation of module registration, dependency validation,
// version compatibility, and lifecycle management.
// ============================================================================

#include "Engine/Runtime/ModuleManager.h"
#include "Engine/Core/Log.h"

#include <queue>
#include <unordered_set>

namespace engine::runtime
{

    // ========================================================================
    // Registration
    // ========================================================================

    bool ModuleManager::Register(std::unique_ptr<IModule> module)
    {
        if (!module)
        {
            ENGINE_LOG_WARN("ModuleManager — attempted to register a null module");
            return false;
        }

        const std::string name = std::string(module->GetName());

        // Duplicate detection.
        if (m_modules.find(name) != m_modules.end())
        {
            ENGINE_LOG_ERROR("ModuleManager — module '{}' is already registered (duplicate rejected)", name);
            return false;
        }

        ENGINE_LOG_DEBUG("ModuleManager — registered module '{}' (v{}.{}.{})",
                         name,
                         module->GetVersionMajor(),
                         module->GetVersionMinor(),
                         module->GetVersionPatch());

        m_modules[name] = std::move(module);
        m_initialized[name] = false;
        m_initOrder.clear();

        return true;
    }

    bool ModuleManager::Unregister(std::string_view name)
    {
        const std::string key(name);

        auto it = m_modules.find(key);
        if (it == m_modules.end())
        {
            return false;
        }

        if (m_initialized[key] && it->second)
        {
            ENGINE_LOG_INFO("ModuleManager — shutting down '{}' before unregister", key);
            try
            {
                it->second->Shutdown();
            }
            catch (const std::exception& e)
            {
                ENGINE_LOG_ERROR("ModuleManager — shutdown threw: {}", e.what());
            }
            catch (...)
            {
                ENGINE_LOG_ERROR("ModuleManager — shutdown threw an unknown exception");
            }
        }

        m_modules.erase(it);
        m_initialized.erase(key);
        m_initOrder.clear();

        ENGINE_LOG_DEBUG("ModuleManager — unregistered module '{}'", key);
        return true;
    }

    // ========================================================================
    // Lookup
    // ========================================================================

    IModule* ModuleManager::GetRaw(std::string_view name) const noexcept
    {
        auto it = m_modules.find(std::string(name));
        if (it != m_modules.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    bool ModuleManager::Has(std::string_view name) const noexcept
    {
        return m_modules.find(std::string(name)) != m_modules.end();
    }

    usize ModuleManager::Count() const noexcept
    {
        return m_modules.size();
    }

    std::vector<std::string> ModuleManager::GetNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_modules.size());
        for (const auto& [name, _] : m_modules)
        {
            names.push_back(name);
        }
        return names;
    }

    bool ModuleManager::IsInitialized(std::string_view name) const noexcept
    {
        auto it = m_initialized.find(std::string(name));
        return it != m_initialized.end() && it->second;
    }

    // ========================================================================
    // Version compatibility
    // ========================================================================

    bool ModuleManager::CheckVersionCompatibility(
        std::string_view name, u32 requiredMajor) const
    {
        auto it = m_modules.find(std::string(name));
        if (it == m_modules.end())
        {
            ENGINE_LOG_WARN("ModuleManager — module '{}' not found for version check", name);
            return false;
        }

        const u32 actual = it->second->GetVersionMajor();
        if (actual != requiredMajor)
        {
            ENGINE_LOG_ERROR(
                "ModuleManager — version mismatch for '{}': required major {}, actual major {}",
                name, requiredMajor, actual);
            return false;
        }

        return true;
    }

    // ========================================================================
    // Lifecycle
    // ========================================================================

    bool ModuleManager::ValidateDependencies(std::string& outError) const
    {
        for (const auto& [name, module] : m_modules)
        {
            for (const auto& dep : module->GetDependencies())
            {
                if (m_modules.find(dep) == m_modules.end())
                {
                    outError = "Module '" + name +
                               "' depends on unregistered module '" +
                               dep + "'";
                    return false;
                }
            }
        }
        return true;
    }

    bool ModuleManager::ComputeInitOrder(std::vector<std::string>& outOrder) const
    {
        outOrder.clear();

        // Kahn's algorithm.
        std::unordered_map<std::string, usize> inDegree;
        std::unordered_map<std::string, std::vector<std::string>> dependents;

        for (const auto& [name, _] : m_modules)
        {
            inDegree[name] = 0;
        }

        for (const auto& [name, module] : m_modules)
        {
            for (const auto& dep : module->GetDependencies())
            {
                ++inDegree[name];
                dependents[dep].push_back(name);
            }
        }

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

        return processed == m_modules.size();
    }

    bool ModuleManager::InitializeAll()
    {
        // Validate dependencies.
        std::string depError;
        if (!ValidateDependencies(depError))
        {
            ENGINE_LOG_ERROR("ModuleManager — dependency validation failed: {}", depError);
            return false;
        }

        // Compute init order.
        std::vector<std::string> order;
        if (!ComputeInitOrder(order))
        {
            ENGINE_LOG_ERROR("ModuleManager — dependency cycle detected among modules");
            return false;
        }

        m_initOrder = order;

        // Initialize in order.
        for (const auto& name : order)
        {
            auto it = m_modules.find(name);
            if (it == m_modules.end() || !it->second)
            {
                ENGINE_LOG_WARN("ModuleManager — module '{}' not found during init", name);
                continue;
            }

            if (m_initialized[name])
            {
                ENGINE_LOG_DEBUG("ModuleManager — module '{}' already initialized, skipping", name);
                continue;
            }

            ENGINE_LOG_INFO("ModuleManager — initializing module '{}'", name);

            try
            {
                const bool success = it->second->Initialize();
                if (!success)
                {
                    ENGINE_LOG_ERROR("ModuleManager — module '{}' initialization failed", name);
                    return false;
                }
            }
            catch (const std::exception& e)
            {
                ENGINE_LOG_ERROR("ModuleManager — module '{}' initialization threw: {}", name, e.what());
                return false;
            }
            catch (...)
            {
                ENGINE_LOG_ERROR("ModuleManager — module '{}' initialization threw an unknown exception", name);
                return false;
            }

            m_initialized[name] = true;
            ENGINE_LOG_INFO("ModuleManager — module '{}' initialized successfully", name);
        }

        ENGINE_LOG_INFO("ModuleManager — all {} module(s) initialized", m_modules.size());
        return true;
    }

    void ModuleManager::ShutdownAll()
    {
        if (m_initOrder.empty())
        {
            (void)ComputeInitOrder(m_initOrder);
        }

        for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
        {
            const auto& name = *it;
            auto modIt = m_modules.find(name);
            if (modIt == m_modules.end() || !modIt->second)
            {
                continue;
            }

            if (!m_initialized[name])
            {
                continue;
            }

            ENGINE_LOG_INFO("ModuleManager — shutting down module '{}'", name);

            try
            {
                modIt->second->Shutdown();
            }
            catch (const std::exception& e)
            {
                ENGINE_LOG_ERROR("ModuleManager — module '{}' shutdown threw: {}", name, e.what());
            }
            catch (...)
            {
                ENGINE_LOG_ERROR("ModuleManager — module '{}' shutdown threw an unknown exception", name);
            }

            m_initialized[name] = false;
        }

        ENGINE_LOG_INFO("ModuleManager — all modules shut down");
    }

} // namespace engine::runtime