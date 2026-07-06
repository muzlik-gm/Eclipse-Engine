// ============================================================================
// File: Engine/Include/Engine/World/World.h
// ISubsystem that manages multiple scenes and selects the active one.
// ============================================================================
#pragma once

#include "Engine/Scene/Scene.h"
#include "Engine/Runtime/ISubsystem.h"
#include "Engine/Events/EventBus.h"
#include "Engine/WorldEvents/WorldEvents.h"

#include <memory>
#include <unordered_map>

namespace engine::world {

    using engine::core::f64;
    using engine::core::usize;
    using engine::events::EventBus;

    // ========================================================================
    // World
    // ========================================================================

    /// @brief Engine subsystem that owns all Scene instances and tracks
    ///        the currently active scene.
    ///
    /// Implements the ISubsystem interface so it participates in the
    /// engine's standard lifecycle (Initialize / Shutdown / Update /
    /// FixedUpdate / LateUpdate).
    class World final : public runtime::ISubsystem
    {
    public:
        World() = default;
        ~World() override = default;

        World(const World&)            = delete;
        World& operator=(const World&) = delete;
        World(World&&)                 = delete;
        World& operator=(World&&)      = delete;

        // -- ISubsystem interface ----------------------------------------------

        [[nodiscard]] std::string_view GetName() const noexcept override;

        [[nodiscard]] std::vector<std::string> GetDependencies() const override;

        bool Initialize() override;
        void Shutdown() override;

        void Update(f64 deltaTime) override;
        void FixedUpdate(f64 fixedDeltaTime) override;
        void LateUpdate(f64 deltaTime) override;

        // -- EventBus binding ---------------------------------------------------

        /// @brief Binds an EventBus so that world and scene events are
        ///        dispatched.  Must be called before Initialize().
        void SetEventBus(EventBus* bus) noexcept { m_EventBus = bus; }

        // -- Scene management --------------------------------------------------

        /// @brief Creates a new scene with the given name and returns a
        ///        reference to it.
        scene::Scene& CreateScene(const std::string& name);

        /// @brief Destroys the scene identified by @p uuid.
        ///
        /// If the destroyed scene was active, the active scene is changed
        /// to the first remaining scene (or nullptr if none exist).
        void DestroyScene(const core::UUID& uuid);

        /// @brief Returns a pointer to the scene with the given UUID,
        ///        or nullptr if not found.
        scene::Scene* GetScene(const core::UUID& uuid);

        /// @brief Returns a pointer to the currently active scene,
        ///        or nullptr if no scene is active.
        scene::Scene* GetActiveScene() { return m_activeScene; }

        /// @brief Sets the active scene to the one identified by @p uuid.
        ///
        /// If no scene with that UUID exists the call is a no-op.
        void SetActiveScene(const core::UUID& uuid);

        // -- Queries ------------------------------------------------------------

        /// @brief Returns the number of scenes currently managed.
        [[nodiscard]] usize SceneCount() const { return m_scenes.size(); }

    private:
        std::unordered_map<core::UUID, std::unique_ptr<scene::Scene>> m_scenes;
        scene::Scene* m_activeScene{nullptr};
        EventBus*     m_EventBus{nullptr};
    };

} // namespace engine::world
