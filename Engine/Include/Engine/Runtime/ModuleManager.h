// ============================================================================
// File: Engine/Include/Engine/Runtime/ModuleManager.h
// Module registration, dependency validation, initialization ordering,
// shutdown ordering, and version compatibility interfaces.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Runtime/IModule.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine::runtime
{

    using engine::core::u32;
    using engine::core::usize;

    // ========================================================================
    // ModuleManager
    // ========================================================================

    /// Manages engine modules — their registration, dependency validation,
    /// version compatibility checks, and lifecycle ordering.
    ///
    /// The ModuleManager operates at a higher level than the SubsystemManager.
    /// Modules register subsystems; the ModuleManager ensures modules load
    /// in the correct dependency order before their subsystems are
    /// initialized.
    ///
    /// Duplicate module names are rejected.  Dependency cycles are detected
    /// at initialization time.  Version compatibility can be checked
    /// explicitly before initialization.
    class ModuleManager
    {
    public:
        ModuleManager()  = default;
        ~ModuleManager() = default;

        ModuleManager(const ModuleManager&)            = delete;
        ModuleManager& operator=(const ModuleManager&) = delete;
        ModuleManager(ModuleManager&&)                 = delete;
        ModuleManager& operator=(ModuleManager&&)      = delete;

        // ----------------------------------------------------------------
        // Registration
        // ----------------------------------------------------------------

        /// Registers a module.  Ownership is transferred.
        /// Returns false if a module with the same name is already
        /// registered (duplicate detection).
        [[nodiscard]] bool Register(std::unique_ptr<IModule> module);

        /// Removes a module by name.  Shuts it down first if initialized.
        /// Returns true if the module existed and was removed.
        bool Unregister(std::string_view name);

        // ----------------------------------------------------------------
        // Lookup
        // ----------------------------------------------------------------

        /// Returns a typed pointer to the named module, or nullptr.
        template <typename T>
        [[nodiscard]] T* Get(std::string_view name) const noexcept
        {
            auto* raw = GetRaw(name);
            return static_cast<T*>(raw);
        }

        /// Returns a raw IModule pointer, or nullptr.
        [[nodiscard]] IModule* GetRaw(std::string_view name) const noexcept;

        /// Returns true if a module with the given name is registered.
        [[nodiscard]] bool Has(std::string_view name) const noexcept;

        /// Returns the number of registered modules.
        [[nodiscard]] usize Count() const noexcept;

        /// Returns all registered module names.
        [[nodiscard]] std::vector<std::string> GetNames() const;

        /// Returns true if the named module has been initialized.
        [[nodiscard]] bool IsInitialized(std::string_view name) const noexcept;

        // ----------------------------------------------------------------
        // Version compatibility
        // ----------------------------------------------------------------

        /// Checks whether the named module's version is compatible with
        /// the requested major version.  A module is compatible when its
        /// major version matches exactly.
        [[nodiscard]] bool CheckVersionCompatibility(
            std::string_view name, u32 requiredMajor) const;

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------

        /// Validates all dependencies and initializes modules in
        /// topological order.  Returns false if a cycle is detected or
        /// any module fails to initialize.
        bool InitializeAll();

        /// Shuts down all initialized modules in reverse order.
        void ShutdownAll();

    private:
        // ----------------------------------------------------------------
        // Internal helpers
        // ----------------------------------------------------------------

        /// Validates that every declared dependency refers to a registered
        /// module.
        [[nodiscard]] bool ValidateDependencies(std::string& outError) const;

        /// Computes a topological ordering of modules.  Returns false if
        /// a cycle is detected.
        [[nodiscard]] bool ComputeInitOrder(
            std::vector<std::string>& outOrder) const;

        // ----------------------------------------------------------------
        // Data
        // ----------------------------------------------------------------

        std::unordered_map<std::string, std::unique_ptr<IModule>> m_modules;
        std::unordered_map<std::string, bool> m_initialized;
        std::vector<std::string> m_initOrder;
    };

} // namespace engine::runtime