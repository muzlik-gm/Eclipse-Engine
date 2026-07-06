// ============================================================================
// File: Engine/Source/Renderer/Graph/RenderGraph.cpp
// ============================================================================
#include "Engine/Renderer/Graph/RenderGraph.h"
#include "Engine/Core/Log.h"

namespace engine::renderer {

    void RenderGraph::RegisterPass(std::unique_ptr<IRenderPass> pass)
    {
        if (!pass)
        {
            ENGINE_LOG_ERROR("RenderGraph — cannot register null pass");
            return;
        }
        ENGINE_LOG_INFO("RenderGraph — registered pass '{}'", pass->GetName());
        m_Passes.push_back(std::move(pass));
        m_Sorted = false;
    }

    void RenderGraph::UnregisterPass(std::string_view name)
    {
        m_Passes.erase(
            std::remove_if(m_Passes.begin(), m_Passes.end(),
                [&](const std::unique_ptr<IRenderPass>& p)
                { return p && p->GetName() == name; }),
            m_Passes.end());
    }

    IRenderPass* RenderGraph::FindPass(std::string_view name) const
    {
        for (const auto& p : m_Passes)
        {
            if (p && p->GetName() == name)
                return p.get();
        }
        return nullptr;
    }

    std::vector<IRenderPass*> RenderGraph::GetPasses() const
    {
        std::vector<IRenderPass*> result;
        result.reserve(m_Passes.size());
        for (const auto& p : m_Passes)
        {
            if (p)
                result.push_back(p.get());
        }
        return result;
    }

    void RenderGraph::InitializeAll()
    {
        SortPasses();
        for (auto& p : m_Passes)
        {
            if (p)
                p->Initialize();
        }
        ENGINE_LOG_INFO("RenderGraph — initialized {} passes", m_Passes.size());
    }

    void RenderGraph::SortPasses()
    {
        std::sort(m_Passes.begin(), m_Passes.end(),
            [](const std::unique_ptr<IRenderPass>& a,
               const std::unique_ptr<IRenderPass>& b)
            {
                return a && b && a->GetSortOrder() < b->GetSortOrder();
            });
        m_Sorted = true;
    }

    void RenderGraph::Execute(const RenderPassContext& context)
    {
        if (!m_Sorted)
            SortPasses();

        m_LastExecuted = 0;
        for (auto& p : m_Passes)
        {
            if (p && p->IsEnabled())
            {
                p->Execute(context);
                ++m_LastExecuted;
            }
        }
    }

    void RenderGraph::ShutdownAll()
    {
        for (auto& p : m_Passes)
        {
            if (p)
                p->Shutdown();
        }
        m_Passes.clear();
        ENGINE_LOG_INFO("RenderGraph — shut down all passes");
    }

} // namespace engine::renderer
