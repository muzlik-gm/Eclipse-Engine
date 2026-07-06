// ============================================================================
// File: Engine/Include/Engine/Runtime/EngineContext.h
// Service locator that provides typed access to engine subsystems and
// shared runtime state.  Subsystems never communicate through globals —
// they query the EngineContext instead.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Events/EventBus.h"
#include "Engine/Runtime/EngineConfig.h"
#include "Engine/Runtime/EngineState.h"
#include "Engine/Runtime/FrameStats.h"
#include "Engine/Runtime/SubsystemManager.h"

#include <any>
#include <mutex>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>

namespace engine::runtime
{

    // ========================================================================
    // EngineContext
    // ========================================================================

    /// Service locator that provides typed access to engine subsystems
    /// and shared runtime state.
    ///
    /// The Engine owns exactly one EngineContext instance and passes
    /// references to subsystems during initialization.  Subsystems use
    /// this context to locate peer subsystems without resorting to
    /// global variables.
    ///
    /// The context also exposes the engine's current state, configuration,
    /// and frame statistics for read-only access.
    class EngineContext
    {
    public:
        EngineContext()  = default;
        ~EngineContext() = default;

        EngineContext(const EngineContext&)            = delete;
        EngineContext& operator=(const EngineContext&) = delete;
        EngineContext(EngineContext&&)                 = delete;
        EngineContext& operator=(EngineContext&&)      = delete;

        // ----------------------------------------------------------------
        // Service registration (called by the Engine during init)
        // ----------------------------------------------------------------

        /// Registers a service by type_index.  Typically used to expose
        /// subsystems by their interface type.
        void RegisterService(std::type_index type, std::any service);

        /// Removes a registered service by type_index.
        void UnregisterService(std::type_index type);

        // ----------------------------------------------------------------
        // Service lookup
        // ----------------------------------------------------------------

        /// Retrieves a service pointer cast to T*, or nullptr if not found.
        template <typename T>
        [[nodiscard]] T* GetService() const
        {
            const std::type_index key = std::type_index(typeid(T));
            auto it = m_services.find(key);
            if (it == m_services.end())
            {
                return nullptr;
            }
            try
            {
                return std::any_cast<T*>(it->second);
            }
            catch (const std::bad_any_cast&)
            {
                return nullptr;
            }
        }

        /// Returns true if a service for the given type is registered.
        template <typename T>
        [[nodiscard]] bool HasService() const
        {
            return m_services.find(std::type_index(typeid(T))) != m_services.end();
        }

        // ----------------------------------------------------------------
        // Direct subsystem access (convenience wrappers)
        // ----------------------------------------------------------------

        /// Access the subsystem manager.
        [[nodiscard]] SubsystemManager& GetSubsystemManager() noexcept
        {
            return m_subsystemManager;
        }

        /// Access the subsystem manager (const).
        [[nodiscard]] const SubsystemManager& GetSubsystemManager() const noexcept
        {
            return m_subsystemManager;
        }

        // ----------------------------------------------------------------
        // Runtime state (read-only for subsystems)
        // ----------------------------------------------------------------

        /// Returns the current engine state.
        [[nodiscard]] EngineState GetState() const noexcept { return m_state; }

        /// Sets the engine state (called by the Engine only).
        void SetState(EngineState state) noexcept { m_state = state; }

        /// Access the engine configuration.
        [[nodiscard]] EngineConfig& GetConfig() noexcept
        {
            return m_config;
        }

        /// Access the engine configuration (const).
        [[nodiscard]] const EngineConfig& GetConfig() const noexcept
        {
            return m_config;
        }

        /// Sets the engine configuration.
        void SetConfig(const EngineConfig& config) noexcept { m_config = config; }

        /// Access frame statistics.
        [[nodiscard]] FrameStats& GetFrameStats() noexcept
        {
            return m_frameStats;
        }

        /// Access frame statistics (const).
        [[nodiscard]] const FrameStats& GetFrameStats() const noexcept
        {
            return m_frameStats;
        }

        // ----------------------------------------------------------------
        // Event bus (shared by all subsystems)
        // ----------------------------------------------------------------

        /// Access the engine-wide event bus.
        [[nodiscard]] events::EventBus& GetEventBus() noexcept
        {
            return m_eventBus;
        }

        /// Access the engine-wide event bus (const).
        [[nodiscard]] const events::EventBus& GetEventBus() const noexcept
        {
            return m_eventBus;
        }

    private:
        SubsystemManager                m_subsystemManager;
        EngineConfig                    m_config;
        EngineState                     m_state = EngineState::Starting;
        FrameStats                      m_frameStats;
        events::EventBus                m_eventBus;
        std::unordered_map<std::type_index, std::any> m_services;
        mutable std::mutex              m_mutex;
    };

} // namespace engine::runtime