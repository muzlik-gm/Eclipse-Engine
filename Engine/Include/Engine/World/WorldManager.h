// ============================================================================
// File: Engine/Include/Engine/World/WorldManager.h
// High-level facade that owns the World.
// ============================================================================
#pragma once

#include "Engine/World/World.h"
#include "Engine/Runtime/ISubsystem.h"
#include "Engine/Events/EventBus.h"

namespace engine::world {

    /// @brief Top-level manager that owns the World subsystem.
    ///
    /// Architecture: Application -> Engine -> WorldManager -> World -> Scene
    class WorldManager final : public runtime::ISubsystem
    {
    public:
        WorldManager() = default;
        ~WorldManager() override = default;

        WorldManager(const WorldManager&)            = delete;
        WorldManager& operator=(const WorldManager&) = delete;
        WorldManager(WorldManager&&)                 = delete;
        WorldManager& operator=(WorldManager&&)      = delete;

        [[nodiscard]] std::string_view GetName() const noexcept override;
        [[nodiscard]] std::vector<std::string> GetDependencies() const override;

        bool Initialize() override;
        void Shutdown() override;

        void Update(f64 deltaTime) override;
        void FixedUpdate(f64 fixedDeltaTime) override;
        void LateUpdate(f64 deltaTime) override;

        /// @brief Binds an EventBus to the World so that world and scene
        ///        events are dispatched.  Must be called before Initialize().
        void SetEventBus(events::EventBus* bus) noexcept { m_world.SetEventBus(bus); }

        [[nodiscard]] World& GetWorld() { return m_world; }
        [[nodiscard]] const World& GetWorld() const { return m_world; }

    private:
        World m_world;
    };

} // namespace engine::world
