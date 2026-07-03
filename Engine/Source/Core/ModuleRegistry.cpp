/**
 * @file ModuleRegistry.cpp
 * @brief Implementation of the engine module registry.
 */

#include "Engine/Core/ModuleRegistry.h"
#include "Engine/Core/Log.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace engine::core {

// ============================================================================
// Singleton
// ============================================================================

ModuleRegistry& ModuleRegistry::Instance()
{
    static ModuleRegistry instance;
    return instance;
}

// ============================================================================
// Registration
// ============================================================================

void ModuleRegistry::Register(const ModuleInfo& module)
{
    std::lock_guard lock(m_mutex);

    // Replace an existing module with the same name, or append.
    for (auto& existing : m_modules)
    {
        if (existing.name == module.name)
        {
            existing.description   = module.description;
            existing.priority      = module.priority;
            existing.initializeFn  = module.initializeFn;
            existing.shutdownFn    = module.shutdownFn;
            existing.initialized   = module.initialized;
            ENGINE_LOG_DEBUG("ModuleRegistry — replaced module '{}'", module.name);
            return;
        }
    }

    m_modules.push_back(module);
    ENGINE_LOG_DEBUG("ModuleRegistry — registered module '{}' (priority {})", module.name, module.priority);
}

void ModuleRegistry::Register(std::string_view name, std::string_view description,
                              u32 priority, std::function<bool()> init,
                              std::function<void()> shutdown)
{
    ModuleInfo info;
    info.name          = std::string(name);
    info.description   = std::string(description);
    info.priority      = priority;
    info.initializeFn  = std::move(init);
    info.shutdownFn    = std::move(shutdown);
    info.initialized   = false;

    Register(info);
}

// ============================================================================
// Lifecycle
// ============================================================================

bool ModuleRegistry::InitializeAll()
{
    std::lock_guard lock(m_mutex);

    // Sort by ascending priority so that low-level modules init first.
    std::vector<ModuleInfo*> sorted;
    sorted.reserve(m_modules.size());
    for (auto& m : m_modules)
    {
        sorted.push_back(&m);
    }

    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const ModuleInfo* a, const ModuleInfo* b)
                     {
                         return a->priority < b->priority;
                     });

    for (auto* mod : sorted)
    {
        if (mod->initialized)
        {
            ENGINE_LOG_DEBUG("ModuleRegistry — module '{}' already initialized, skipping", mod->name);
            continue;
        }

        if (!mod->initializeFn)
        {
            ENGINE_LOG_WARN("ModuleRegistry — module '{}' has no initialize function, skipping", mod->name);
            continue;
        }

        ENGINE_LOG_INFO("ModuleRegistry — initializing '{}' (priority {})", mod->name, mod->priority);

        try
        {
            bool success = mod->initializeFn();
            if (!success)
            {
                ENGINE_LOG_ERROR("ModuleRegistry — module '{}' initialization failed", mod->name);
                return false;
            }
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("ModuleRegistry — module '{}' initialization threw: {}", mod->name, e.what());
            return false;
        }
        catch (...)
        {
            ENGINE_LOG_ERROR("ModuleRegistry — module '{}' initialization threw an unknown exception", mod->name);
            return false;
        }

        mod->initialized = true;
        ENGINE_LOG_INFO("ModuleRegistry — module '{}' initialized successfully", mod->name);
    }

    ENGINE_LOG_INFO("ModuleRegistry — all {} modules initialized", sorted.size());
    return true;
}

void ModuleRegistry::ShutdownAll()
{
    std::lock_guard lock(m_mutex);

    // Sort by descending priority so that high-level modules shut down first.
    std::vector<ModuleInfo*> sorted;
    sorted.reserve(m_modules.size());
    for (auto& m : m_modules)
    {
        sorted.push_back(&m);
    }

    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const ModuleInfo* a, const ModuleInfo* b)
                     {
                         return a->priority > b->priority;
                     });

    for (auto* mod : sorted)
    {
        if (!mod->initialized)
        {
            continue;
        }

        if (!mod->shutdownFn)
        {
            ENGINE_LOG_WARN("ModuleRegistry — module '{}' has no shutdown function", mod->name);
            mod->initialized = false;
            continue;
        }

        ENGINE_LOG_INFO("ModuleRegistry — shutting down '{}' (priority {})", mod->name, mod->priority);

        try
        {
            mod->shutdownFn();
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("ModuleRegistry — module '{}' shutdown threw: {}", mod->name, e.what());
        }
        catch (...)
        {
            ENGINE_LOG_ERROR("ModuleRegistry — module '{}' shutdown threw an unknown exception", mod->name);
        }

        mod->initialized = false;
        ENGINE_LOG_INFO("ModuleRegistry — module '{}' shut down", mod->name);
    }

    ENGINE_LOG_INFO("ModuleRegistry — all modules shut down");
}

// ============================================================================
// Queries
// ============================================================================

bool ModuleRegistry::IsInitialized(std::string_view name) const
{
    std::lock_guard lock(m_mutex);

    for (const auto& m : m_modules)
    {
        if (m.name == name)
        {
            return m.initialized;
        }
    }
    return false;
}

const ModuleInfo* ModuleRegistry::GetModule(std::string_view name) const
{
    std::lock_guard lock(m_mutex);

    for (const auto& m : m_modules)
    {
        if (m.name == name)
        {
            return &m;
        }
    }
    return nullptr;
}

std::vector<const ModuleInfo*> ModuleRegistry::GetAllModules() const
{
    std::lock_guard lock(m_mutex);

    std::vector<const ModuleInfo*> result;
    result.reserve(m_modules.size());
    for (const auto& m : m_modules)
    {
        result.push_back(&m);
    }
    return result;
}

// ============================================================================
// Unregistration
// ============================================================================

void ModuleRegistry::Unregister(std::string_view name)
{
    std::lock_guard lock(m_mutex);

    for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
    {
        if (it->name == name)
        {
            // Shut down the module if it is currently initialized.
            if (it->initialized && it->shutdownFn)
            {
                ENGINE_LOG_INFO("ModuleRegistry — shutting down '{}' before unregister", it->name);
                try
                {
                    it->shutdownFn();
                }
                catch (const std::exception& e)
                {
                    ENGINE_LOG_ERROR("ModuleRegistry — module '{}' shutdown threw during unregister: {}",
                                 it->name, e.what());
                }
                catch (...)
                {
                    ENGINE_LOG_ERROR("ModuleRegistry — module '{}' shutdown threw an unknown exception during unregister",
                                 it->name);
                }
            }

            ENGINE_LOG_DEBUG("ModuleRegistry — unregistered module '{}'", it->name);
            m_modules.erase(it);
            return;
        }
    }
}

} // namespace engine::core