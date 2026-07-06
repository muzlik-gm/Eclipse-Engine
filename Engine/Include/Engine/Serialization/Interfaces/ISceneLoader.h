// ============================================================================
// File: Engine/Include/Engine/Serialization/Interfaces/ISceneLoader.h
// Abstract interface for loading a scene from external storage.
// ============================================================================
#pragma once

#include <memory>
#include <string>

namespace engine::scene {

    class Scene;

    /// @brief Abstract interface for scene loading backends.
    class ISceneLoader
    {
    public:
        virtual ~ISceneLoader() = default;

        /// @brief Loads a scene from the given file path.
        [[nodiscard]] virtual std::unique_ptr<Scene> Load(
            const std::string& filePath) = 0;

        /// @brief Returns true if this loader can handle the given extension.
        [[nodiscard]] virtual bool SupportsExtension(
            const std::string& extension) const = 0;
    };

} // namespace engine::scene
