// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Factories/IGraphicsBackend.h
// Abstract interface for a graphics backend — a pluggable module that
// creates IGraphicsInstance objects and registers itself with the
// backend registry.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsDevice.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace engine::rhi {

    // ========================================================================
    // BackendCreateInfo — parameters used to create a graphics instance.
    // ========================================================================

    /// @brief Parameters used to create an IGraphicsInstance from a backend.
    struct BackendCreateInfo
    {
        GraphicsBackend Backend{GraphicsBackend::None};

        /// Application name forwarded to the driver (for profiling).
        std::string ApplicationName{"Eclipse Engine"};

        /// Application version (major.minor.patch packed into u32).
        u32 ApplicationVersion{0};

        /// Enable backend validation / debug layers.
        bool EnableValidation{true};

        /// Enable debug output (callbacks, markers, labels).
        bool EnableDebugOutput{true};

        /// Required adapter vendor (Unknown = any).
        GraphicsVendor PreferredVendor{GraphicsVendor::Unknown};

        /// Window handle for context creation (platform-specific).
        void* WindowHandle{nullptr};

        /// Initial swapchain dimensions.
        u32 WindowWidth{1280};
        u32 WindowHeight{720};
    };

    // ========================================================================
    // IGraphicsBackend — factory of graphics instances.
    // ========================================================================

    /// @brief A pluggable graphics backend module.  Each backend
    ///        (OpenGL, Vulkan, D3D12, Metal, WebGPU) implements this
    ///        interface and registers itself with GraphicsBackendRegistry.
    ///        The engine queries the registry to enumerate available
    ///        backends and create the requested one.
    class IGraphicsBackend
    {
    public:
        virtual ~IGraphicsBackend() = default;

        /// @brief Returns the backend type.
        [[nodiscard]] virtual GraphicsBackend GetType() const noexcept = 0;

        /// @brief Returns a human-readable name (e.g. "OpenGL 4.6 Core").
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

        /// @brief Returns true if this backend is available on the
        ///        current platform.
        [[nodiscard]] virtual bool IsAvailable() const = 0;

        /// @brief Creates a new graphics instance from this backend.
        [[nodiscard]] virtual std::unique_ptr<IGraphicsInstance>
            CreateInstance(const BackendCreateInfo& info) = 0;
    };

    // ========================================================================
    // GraphicsBackendRegistry — registry of available backends.
    // ========================================================================

    /// @brief Registry of available graphics backends.  Backends register
    ///        themselves at process start; the engine queries the registry
    ///        to find the requested backend.
    class GraphicsBackendRegistry
    {
    public:
        /// @brief Returns the global registry instance.
        [[nodiscard]] static GraphicsBackendRegistry& Get();

        /// @brief Registers a backend.  Called by backend init code.
        void Register(IGraphicsBackend* backend);

        /// @brief Unregisters a backend.
        void Unregister(IGraphicsBackend* backend);

        /// @brief Returns the backend of the given type, or nullptr.
        [[nodiscard]] IGraphicsBackend* GetBackend(GraphicsBackend type) const;

        /// @brief Enumerates all registered backends.
        [[nodiscard]] std::vector<IGraphicsBackend*> EnumerateBackends() const;

        /// @brief Returns the default backend (the first registered one).
        [[nodiscard]] IGraphicsBackend* GetDefaultBackend() const;

    private:
        GraphicsBackendRegistry() = default;
        std::vector<IGraphicsBackend*> m_Backends;
    };

    // ========================================================================
    // BackendRegistrar — RAII helper for backend self-registration.
    // ========================================================================

    /// @brief RAII helper that registers a backend on construction and
    ///        unregisters it on destruction.  Used by backend modules:
    ///
    /// @code
    ///   static OpenGLBackend g_openglBackend;
    ///   static rhi::BackendRegistrar g_registrar(&g_openglBackend);
    /// @endcode
    class BackendRegistrar
    {
    public:
        explicit BackendRegistrar(IGraphicsBackend* backend)
            : m_Backend(backend)
        {
            if (m_Backend)
                GraphicsBackendRegistry::Get().Register(m_Backend);
        }

        ~BackendRegistrar()
        {
            if (m_Backend)
                GraphicsBackendRegistry::Get().Unregister(m_Backend);
        }

        BackendRegistrar(const BackendRegistrar&)            = delete;
        BackendRegistrar& operator=(const BackendRegistrar&) = delete;
        BackendRegistrar(BackendRegistrar&&)                 = delete;
        BackendRegistrar& operator=(BackendRegistrar&&)      = delete;

    private:
        IGraphicsBackend* m_Backend;
    };

} // namespace engine::rhi
