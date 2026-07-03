// ============================================================================
// File: Engine/Include/Engine/Memory/Allocator.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <cstddef>

namespace engine::memory {

    using engine::core::usize;

    /// Strategy used by an allocator implementation.
    enum class AllocationStrategy
    {
        System,  ///< Default OS heap allocator (malloc / new)
        Pool,    ///< Fixed-size block pool
        Stack,   ///< Stack-like LIFO allocator
        Arena,   ///< Bump / arena allocator
        Linear   ///< Linear scan allocator
    };

    /// Pure-virtual memory allocator interface.
    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;

        /// Allocate `size` bytes with the given `alignment`.
        /// Returns a pointer to the allocated block, or nullptr on failure.
        [[nodiscard]] virtual void* Allocate(usize size, usize alignment = 8) = 0;

        /// Deallocate a previously allocated block.
        virtual void Deallocate(void* ptr) = 0;

        /// Total number of bytes currently allocated through this allocator.
        [[nodiscard]] virtual usize GetTotalAllocated() const = 0;

        /// Number of outstanding allocation calls (allocations not yet freed).
        [[nodiscard]] virtual usize GetAllocationCount() const = 0;

        /// Reset the allocator state, releasing or recycling all memory.
        virtual void Reset() = 0;
    };

} // namespace engine::memory