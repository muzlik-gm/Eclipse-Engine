// ============================================================================
// File: Engine/Include/Engine/Runtime/SubsystemManager.h
// Registration, lookup, dependency-ordered initialization, and per-frame
// updates for all engine subsystems.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Runtime/ISubsystem.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine::runtime
{

    using engine::core::f64;
    using engine::core::usize;

    // ========================================================================
    // SubsystemManager
    // ========================================================================

    /// Manages the registration, lifecycle, and per-frame updates of all
    /// engine subsystems.
    ///
    /// Subsystems are registered by name before engine initialization.  The
    /// manager topologically sorts them based on declared dependencies,
    /// detects cycles, and initializes them in the computed order.
    /// Shutdown occurs in reverse order.
    ///
    /// The manager does NOT use global variables — subsystems communicate
    /// through the EngineContext (service locator) passed during init.
    class SubsystemManager
    {
    public:
        SubsystemManager()  = default;
        ~SubsystemManager() = default;

        SubsystemManager(const SubsystemManager&)            = delete;
        SubsystemManager& operator=(const SubsystemManager&) = delete;
        SubsystemManager(SubsystemManager&&)                 = delete;
        SubsystemManager& operator=(SubsystemManager&&)      = delete;

        // ----------------------------------------------------------------
        // Registration
        // ----------------------------------------------------------------

        /// Registers a subsystem.  Ownership is transferred to the manager.
        /// If a subsystem with the same name already exists, the existing
        /// one is replaced (shut down first if initialized).
        void Register(std::unique_ptr<ISubsystem> subsystem);

        /// Removes a subsystem by name.  Shuts it down first if initialized.
        /// Returns true if the subsystem existed and was removed.
        bool Unregister(std::string_view name);

        // ----------------------------------------------------------------
        // Lookup
        // ----------------------------------------------------------------

        /// Returns a typed pointer to the named subsystem, or nullptr.
        template <typename T>
        [[nodiscard]] T* Get(std::string_view name) const noexcept
        {
            auto* raw = GetRaw(name);
            return static_cast<T*>(raw);
        }

        /// Returns a raw ISubsystem pointer, or nullptr.
        [[nodiscard]] ISubsystem* GetRaw(std::string_view name) const noexcept;

        /// Returns true if a subsystem with the given name is registered.
        [[nodiscard]] bool Has(std::string_view name) const noexcept;

        /// Returns the number of registered subsystems.
        [[nodiscard]] usize Count() const noexcept;

        /// Returns all registered subsystem names.
        [[nodiscard]] std::vector<std::string> GetNames() const;

        /// Returns true if the named subsystem has been initialized.
        [[nodiscard]] bool IsInitialized(std::string_view name) const noexcept;

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------

        /// Computes the initialization order (topological sort based on
        /// declared dependencies), then initializes all subsystems in that
        /// order.  Stops on the first failure.
        ///
        /// Returns true if all subsystems initialized successfully.
        bool InitializeAll();

        /// Shuts down all initialized subsystems in reverse initialization
        /// order.  Safely handles partial initialization.
        void ShutdownAll();

        // ----------------------------------------------------------------
        // Per-frame updates
        // ----------------------------------------------------------------

        /// Calls Update(deltaTime) on every initialized subsystem in init
        /// order.
        void UpdateAll(f64 deltaTime);

        /// Calls FixedUpdate(fixedDeltaTime) on every initialized subsystem
        /// in init order.
        void FixedUpdateAll(f64 fixedDeltaTime);

        /// Calls LateUpdate(deltaTime) on every initialized subsystem in
        /// init order.
        void LateUpdateAll(f64 deltaTime);

    private:
        // ----------------------------------------------------------------
        // Internal helpers
        // ----------------------------------------------------------------

        /// Computes a topological ordering of subsystems.  Returns false
        /// if a dependency cycle is detected.
        [[nodiscard]] bool ComputeInitOrder(
            std::vector<std::string>& outOrder) const;

        /// Validates that every declared dependency refers to a registered
        /// subsystem.
        [[nodiscard]] bool ValidateDependencies(std::string& outError) const;

        // ----------------------------------------------------------------
        // Data
        // ----------------------------------------------------------------

        /// Stores subsystems by name.
        std::unordered_map<std::string, std::unique_ptr<ISubsystem>> m_subsystems;

        /// Tracks which subsystems have been successfully initialized.
        std::unordered_map<std::string, bool> m_initialized;

        /// Cached initialization order (populated by ComputeInitOrder).
        std::vector<std::string> m_initOrder;
    };

} // namespace engine::runtime