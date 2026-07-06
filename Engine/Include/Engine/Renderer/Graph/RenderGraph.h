// ============================================================================
// File: Engine/Include/Engine/Renderer/Graph/RenderGraph.h
// Render graph — manages pass registration, dependencies, and execution.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/Passes/IRenderPass.h"

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine::renderer {

    // ========================================================================
    // RenderGraph — manages the execution order of render passes.
    // ========================================================================

    /// @brief The Render Graph manages the lifecycle of render passes.
    ///        Passes are registered with a sort order, and the graph
    ///        executes them in order.  Future extensions will add
    ///        resource dependency tracking and automatic reordering.
    class RenderGraph
    {
    public:
        RenderGraph() = default;
        ~RenderGraph() = default;

        /// @brief Registers a render pass.  Takes ownership.
        void RegisterPass(std::unique_ptr<IRenderPass> pass);

        /// @brief Removes a pass by name.
        void UnregisterPass(std::string_view name);

        /// @brief Finds a pass by name.
        [[nodiscard]] IRenderPass* FindPass(std::string_view name) const;

        /// @brief Returns all registered passes.
        [[nodiscard]] std::vector<IRenderPass*> GetPasses() const;

        /// @brief Initializes all passes (compiles shaders, creates geometry).
        void InitializeAll();

        /// @brief Sorts passes by sort order.
        void SortPasses();

        /// @brief Executes all enabled passes in sorted order.
        void Execute(const RenderPassContext& context);

        /// @brief Shuts down all passes (releases GPU resources).
        void ShutdownAll();

        /// @brief Returns the number of registered passes.
        [[nodiscard]] core::u32 GetPassCount() const noexcept
        { return static_cast<core::u32>(m_Passes.size()); }

        /// @brief Returns the number of passes executed in the last frame.
        [[nodiscard]] core::u32 GetLastExecutedCount() const noexcept
        { return m_LastExecuted; }

    private:
        std::vector<std::unique_ptr<IRenderPass>> m_Passes;
        core::u32                                 m_LastExecuted{0};
        bool                                      m_Sorted{false};
    };

} // namespace engine::renderer
