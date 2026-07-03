// ============================================================================
// File: Engine/Include/Engine/Runtime/EngineState.h
// Runtime state machine — defines the lifecycle states and valid transitions
// for the engine runtime.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"

#include <array>
#include <string_view>

namespace engine::runtime
{

    using engine::core::u8;

    // ========================================================================
    // EngineState
    // ========================================================================

    /// Enumerates every state the engine runtime can occupy.
    ///
    /// The state machine is intentionally linear for Phase 1 — more complex
    /// branching (e.g. hot-reload) may be introduced in later phases via
    /// additional transitions.
    enum class EngineState : u8
    {
        /// The engine object has been created but Initialize() has not been
        /// called yet.
        Starting     = 0,

        /// Subsystems and modules are being initialized.
        Initializing = 1,

        /// The main loop is running and ticking subsystems.
        Running      = 2,

        /// The main loop is paused — FixedUpdate still runs but Update and
        /// LateUpdate are skipped.
        Paused       = 3,

        /// The engine is performing its shutdown sequence.
        Stopping     = 4,

        /// The engine has been fully shut down and cannot be restarted.
        Shutdown     = 5
    };

    // ========================================================================
    // EngineStateInfo — state metadata
    // ========================================================================

    /// Returns a human-readable name for the given engine state.
    [[nodiscard]] constexpr std::string_view EngineStateToString(EngineState state) noexcept
    {
        switch (state)
        {
            case EngineState::Starting:     return "Starting";
            case EngineState::Initializing: return "Initializing";
            case EngineState::Running:      return "Running";
            case EngineState::Paused:       return "Paused";
            case EngineState::Stopping:     return "Stopping";
            case EngineState::Shutdown:     return "Shutdown";
        }
        return "Unknown";
    }

    // ========================================================================
    // StateMachine — validates transitions between engine states.
    // ========================================================================

    /// Validates whether a transition from @p from to @p to is legal.
    ///
    /// The legal transition table is:
    ///
    ///   From \ To   | Starting | Initializing | Running | Paused | Stopping | Shutdown
    ///   -------------|----------|--------------|---------|--------|----------|--------
    ///   Starting     |          |    YES       |         |        |          |
    ///   Initializing |          |              |   YES   |        |   YES*   |
    ///   Running      |          |              |         |  YES   |   YES    |
    ///   Paused       |          |              |   YES   |        |   YES    |
    ///   Stopping     |          |              |         |        |          |   YES
    ///   Shutdown     |          |              |         |        |          |
    ///
    ///   *YES* = initialization failure causes early transition to Stopping.
    [[nodiscard]] constexpr bool IsEngineStateTransitionValid(
        EngineState from, EngineState to) noexcept
    {
        // Same state is never a valid "transition".
        if (from == to)
        {
            return false;
        }

        switch (from)
        {
            case EngineState::Starting:
                return to == EngineState::Initializing ||
                       to == EngineState::Stopping;

            case EngineState::Initializing:
                return to == EngineState::Running ||
                       to == EngineState::Stopping;

            case EngineState::Running:
                return to == EngineState::Paused ||
                       to == EngineState::Stopping;

            case EngineState::Paused:
                return to == EngineState::Running ||
                       to == EngineState::Stopping;

            case EngineState::Stopping:
                return to == EngineState::Shutdown;

            case EngineState::Shutdown:
                return false;
        }

        return false;
    }

} // namespace engine::runtime