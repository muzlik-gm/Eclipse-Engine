// ============================================================================
// File: Engine/Source/World/WorldManager.cpp
// ============================================================================
#include "Engine/World/WorldManager.h"

namespace engine::world {

    std::string_view WorldManager::GetName() const noexcept { return "WorldManager"; }
    std::vector<std::string> WorldManager::GetDependencies() const { return {}; }

    bool WorldManager::Initialize() { return m_world.Initialize(); }
    void WorldManager::Shutdown() { m_world.Shutdown(); }
    void WorldManager::Update(f64 deltaTime) { m_world.Update(deltaTime); }
    void WorldManager::FixedUpdate(f64 fixedDeltaTime) { m_world.FixedUpdate(fixedDeltaTime); }
    void WorldManager::LateUpdate(f64 deltaTime) { m_world.LateUpdate(deltaTime); }

} // namespace engine::world
