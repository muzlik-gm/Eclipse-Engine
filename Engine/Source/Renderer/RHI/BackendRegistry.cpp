// ============================================================================
// File: Engine/Source/Renderer/RHI/BackendRegistry.cpp
// Implementation of GraphicsBackendRegistry.
// ============================================================================
#include "Engine/Renderer/RHI/Factories/IGraphicsBackend.h"
#include "Engine/Core/Log.h"

namespace engine::rhi {

    GraphicsBackendRegistry& GraphicsBackendRegistry::Get()
    {
        static GraphicsBackendRegistry instance;
        return instance;
    }

    void GraphicsBackendRegistry::Register(IGraphicsBackend* backend)
    {
        if (!backend)
            return;

        // Check for duplicates.
        for (auto* b : m_Backends)
        {
            if (b == backend || b->GetType() == backend->GetType())
            {
                ENGINE_LOG_DEBUG("GraphicsBackendRegistry — backend '{}' already registered",
                                 GraphicsBackendToString(backend->GetType()));
                return;
            }
        }

        ENGINE_LOG_INFO("GraphicsBackendRegistry — registered backend '{}'",
                        GraphicsBackendToString(backend->GetType()));
        m_Backends.push_back(backend);
    }

    void GraphicsBackendRegistry::Unregister(IGraphicsBackend* backend)
    {
        if (!backend)
            return;

        m_Backends.erase(
            std::remove(m_Backends.begin(), m_Backends.end(), backend),
            m_Backends.end());

        ENGINE_LOG_DEBUG("GraphicsBackendRegistry — unregistered backend '{}'",
                         GraphicsBackendToString(backend->GetType()));
    }

    IGraphicsBackend* GraphicsBackendRegistry::GetBackend(GraphicsBackend type) const
    {
        for (auto* b : m_Backends)
        {
            if (b && b->GetType() == type)
                return b;
        }
        return nullptr;
    }

    std::vector<IGraphicsBackend*> GraphicsBackendRegistry::EnumerateBackends() const
    {
        return m_Backends;
    }

    IGraphicsBackend* GraphicsBackendRegistry::GetDefaultBackend() const
    {
        if (!m_Backends.empty())
            return m_Backends.front();
        return nullptr;
    }

} // namespace engine::rhi
