// ============================================================================
// File: Engine/Include/Engine/Runtime/IModule.h
// Interface that every engine module must implement for the ModuleManager.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"

#include <string>
#include <string_view>
#include <vector>

namespace engine::runtime
{

    using engine::core::u32;

    // ========================================================================
    // IModule
    // ========================================================================

    /// Pure virtual interface for engine modules.
    ///
    /// Modules are coarser-grained than subsystems — a module may own one or
    /// more subsystems and is responsible for registering them with the
    /// SubsystemManager.  The ModuleManager validates dependencies between
    /// modules and controls their init/shutdown order.
    ///
    /// This interface also carries version information for future
    /// compatibility checks.
    class IModule
    {
    public:
        virtual ~IModule() = default;

        // ----------------------------------------------------------------
        // Identity
        // ----------------------------------------------------------------

        /// Returns the unique name of this module.
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

        /// Human-readable description.
        [[nodiscard]] virtual std::string_view GetDescription() const noexcept = 0;

        // ----------------------------------------------------------------
        // Version
        // ----------------------------------------------------------------

        /// Major version number.
        [[nodiscard]] virtual u32 GetVersionMajor() const noexcept { return 0; }

        /// Minor version number.
        [[nodiscard]] virtual u32 GetVersionMinor() const noexcept { return 0; }

        /// Patch version number.
        [[nodiscard]] virtual u32 GetVersionPatch() const noexcept { return 0; }

        // ----------------------------------------------------------------
        // Dependencies
        // ----------------------------------------------------------------

        /// Returns the names of modules that must be loaded before this one.
        [[nodiscard]] virtual std::vector<std::string> GetDependencies() const
        {
            return {};
        }

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------

        /// Called during engine initialization.  Modules should register
        /// their subsystems here.  Return true on success.
        virtual bool Initialize() = 0;

        /// Called during engine shutdown.
        virtual void Shutdown() = 0;
    };

} // namespace engine::runtime