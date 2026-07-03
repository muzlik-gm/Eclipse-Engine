// ============================================================================
// File: Engine/Include/Engine/Platform/DynamicLibrary.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <memory>
#include <string>

namespace engine::platform {

    // ========================================================================
    // IDynamicLibrary – pure-virtual dynamic library (shared object) interface
    // ========================================================================
    class IDynamicLibrary : private engine::core::NonCopyable
    {
    public:
        virtual ~IDynamicLibrary() = default;

        /// Loads a shared library from the given file path.
        /// Returns true on success, false if the library could not be loaded.
        virtual bool Load(const std::string& filePath) = 0;

        /// Unloads the shared library. Safe to call even if not loaded.
        virtual void Unload() = 0;

        /// Retrieves a symbol (function or variable) by name.
        /// Returns nullptr if the symbol is not found or the library is not loaded.
        [[nodiscard]] virtual void* GetSymbol(const std::string& name) const = 0;

        /// Returns true if a shared library is currently loaded.
        [[nodiscard]] virtual bool IsLoaded() const = 0;

        /// Returns the file path that was used to load the library.
        /// Returns an empty string if no library is loaded.
        [[nodiscard]] virtual const std::string& GetFilePath() const = 0;

        // --------------------------------------------------------------------
        // Factory – creates the platform-specific implementation.
        // --------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<IDynamicLibrary> Create();
    };

} // namespace engine::platform