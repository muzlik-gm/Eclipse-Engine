// ============================================================================
// File: Engine/Include/Engine/Serialization/Interfaces/ISceneSaver.h
// Abstract interface for saving a scene to external storage.
// ============================================================================
#pragma once

#include <string>

namespace engine::scene {

    class Scene;

    /// @brief Abstract interface for scene saving backends.
    class ISceneSaver
    {
    public:
        virtual ~ISceneSaver() = default;

        virtual bool Save(const Scene& scene, const std::string& filePath) = 0;

        [[nodiscard]] virtual std::string GetDefaultExtension() const = 0;
    };

} // namespace engine::scene
