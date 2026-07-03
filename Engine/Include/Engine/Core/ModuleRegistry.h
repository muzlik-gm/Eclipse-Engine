#pragma once

/**
 * @file ModuleRegistry.h
 * @brief Infrastructure for registering engine modules and managing their
 *        initialization and shutdown order.
 *
 * Each module is described by a ModuleInfo struct that carries a name,
 * human-readable description, numeric priority, and init/shutdown function
 * pointers.  Modules are initialized in ascending priority order and shut
 * down in descending priority order, ensuring that higher-level subsystems
 * are torn down before the low-level services they depend on.
 *
 * The registry is a thread-safe singleton accessed via Instance().
 */

#include "Engine/Core/Types.h"

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace engine::core {

/// Describes a single engine module for registration with the registry.
struct ModuleInfo
{
    /// Unique name used to identify and look up the module.
    std::string name;

    /// Human-readable description of what the module provides.
    std::string description;

    /**
     * Initialization priority.  Lower values are initialized first;
     * higher values are shut down first.  Typical ranges:
     *   0–99   : Platform / low-level services
     *   100–199: Core subsystems (logging, memory, config)
     *   200–299: Mid-level systems (rendering, audio, physics)
     *   300+   : High-level / gameplay systems
     */
    u32 priority = 0;

    /// Called during InitializeAll().  Must return true on success.
    std::function<bool()> initializeFn;

    /// Called during ShutdownAll().
    std::function<void()> shutdownFn;

    /// Set to true after successful initialization.
    bool initialized = false;
};

/**
 * @brief Singleton registry for engine modules.
 *
 * Provides a centralized point for registering subsystems and executing
 * their lifecycle callbacks in a well-defined priority order.
 */
class ModuleRegistry
{
public:
    /// Returns the singleton instance.
    static ModuleRegistry& Instance();

    // Non-copyable, non-movable.
    ModuleRegistry(const ModuleRegistry&)            = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;
    ModuleRegistry(ModuleRegistry&&)                 = delete;
    ModuleRegistry& operator=(ModuleRegistry&&)      = delete;

    /**
     * @brief Registers a module described by @p module.
     *
     * If a module with the same name already exists it is replaced.
     */
    void Register(const ModuleInfo& module);

    /**
     * @brief Convenience overload that constructs a ModuleInfo in-place.
     *
     * @param name        Unique module name.
     * @param description Human-readable description.
     * @param priority    Initialization priority (lower = first).
     * @param init        Initialization function (must return true on success).
     * @param shutdown    Shutdown function.
     */
    void Register(std::string_view name, std::string_view description,
                  u32 priority, std::function<bool()> init,
                  std::function<void()> shutdown);

    /**
     * @brief Initializes all registered modules in ascending priority order.
     *
     * Stops on the first module whose initializeFn returns false (or throws).
     * Modules initialized before the failure are left in their initialized
     * state so that a subsequent ShutdownAll() can tear them down cleanly.
     *
     * @return True when every module initialized successfully.
     */
    bool InitializeAll();

    /**
     * @brief Shuts down all initialized modules in descending priority order.
     *
     * Safely handles partial initialization — only modules that were
     * successfully initialized are shut down.
     */
    void ShutdownAll();

    /**
     * @brief Returns true if the named module has been initialized.
     */
    [[nodiscard]] bool IsInitialized(std::string_view name) const;

    /**
     * @brief Returns a pointer to the named module's ModuleInfo, or nullptr.
     */
    [[nodiscard]] const ModuleInfo* GetModule(std::string_view name) const;

    /**
     * @brief Returns pointers to all registered modules (unsorted).
     */
    [[nodiscard]] std::vector<const ModuleInfo*> GetAllModules() const;

    /**
     * @brief Removes a module from the registry by name.
     *
     * If the module is currently initialized, it is shut down first.
     */
    void Unregister(std::string_view name);

private:
    ModuleRegistry() = default;

    std::vector<ModuleInfo> m_modules;
    mutable std::mutex m_mutex;
};

} // namespace engine::core