// ============================================================================
// File: Engine/Include/Engine/Renderer/Queue/RenderQueue.h
// Render queue that batches and sorts draw calls by type.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/ECS/Entity.h"

#include <vector>
#include <functional>

namespace engine::renderer {

    using engine::core::u32;
    using engine::core::f32;
    using engine::math::Mat4;
    using engine::ecs::Entity;

    // ========================================================================
    // RenderQueueType — identifies which queue a command belongs to.
    // ========================================================================

    enum class RenderQueueType : u32
    {
        Opaque      = 0,
        Transparent = 1,
        Overlay     = 2,
        Editor      = 3,
        Debug       = 4,
        UI          = 5,
        Custom      = 100
    };

    // ========================================================================
    // RenderQueueEntry — a single submission to the render queue.
    // ========================================================================

    struct RenderQueueEntry
    {
        Entity          EntityHandle{engine::ecs::Invalid};
        Mat4            WorldMatrix{1.0f};
        Mat4            ViewProjection{1.0f};
        u32             MeshType{0};
        u32             SortKey{0};
        f32             Distance{0.0f};    // for sorting
        bool            CastShadows{true};
        bool            Visible{true};

        // Sorting: opaque = front-to-back, transparent = back-to-front.
        [[nodiscard]] bool operator<(const RenderQueueEntry& other) const noexcept
        {
            return SortKey < other.SortKey;
        }
    };

    // ========================================================================
    // RenderQueue — holds and sorts draw submissions.
    // ========================================================================

    /// @brief A render queue collects draw submissions, sorts them
    ///        (front-to-back for opaque, back-to-front for transparent),
    ///        and provides iteration for the render passes.
    class RenderQueue
    {
    public:
        RenderQueue() = default;
        ~RenderQueue() = default;

        /// @brief Clears all entries.
        void Clear() { m_Entries.clear(); }

        /// @brief Submits a draw entry to this queue.
        void Submit(const RenderQueueEntry& entry) { m_Entries.push_back(entry); }

        /// @brief Sorts entries by sort key.
        void Sort();

        /// @brief Returns the number of entries.
        [[nodiscard]] u32 GetCount() const noexcept
        { return static_cast<u32>(m_Entries.size()); }

        /// @brief Returns all entries (const).
        [[nodiscard]] const std::vector<RenderQueueEntry>& GetEntries() const noexcept
        { return m_Entries; }

        /// @brief Iterates over all entries, calling @p callback for each.
        void ForEach(const std::function<void(const RenderQueueEntry&)>& callback) const;

    private:
        std::vector<RenderQueueEntry> m_Entries;
    };

    // ========================================================================
    // RenderQueueManager — owns all render queues.
    // ========================================================================

    /// @brief Manages all render queues.  Each frame, systems submit
    ///        draw calls to the appropriate queue; render passes then
    ///        iterate the queues to issue draw commands.
    class RenderQueueManager
    {
    public:
        RenderQueueManager();
        ~RenderQueueManager() = default;

        /// @brief Clears all queues.  Call at the start of each frame.
        void ClearAll();

        /// @brief Returns the queue for @p type.
        [[nodiscard]] RenderQueue& GetQueue(RenderQueueType type);

        /// @brief Sorts all queues.
        void SortAll();

        /// @brief Submits an entry to the specified queue.
        void Submit(RenderQueueType type, const RenderQueueEntry& entry);

        /// @brief Returns total entry count across all queues.
        [[nodiscard]] u32 GetTotalCount() const noexcept;

    private:
        RenderQueue m_Queues[6]; // Opaque, Transparent, Overlay, Editor, Debug, UI
    };

} // namespace engine::renderer
