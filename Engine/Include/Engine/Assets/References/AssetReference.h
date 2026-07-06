// ============================================================================
// File: Engine/Include/Engine/Assets/References/AssetReference.h
// Strong and weak references to assets with reference counting.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"

#include <atomic>
#include <memory>
#include <mutex>

namespace engine::assets {

    class IAsset;
    class AssetManager;

    // ========================================================================
    // AssetControlBlock — reference-counted control block.
    // ========================================================================

    /// @brief Control block shared between strong and weak references.
    ///        Tracks the strong reference count, weak reference count,
    ///        and the asset's current state.
    struct AssetControlBlock
    {
        std::atomic<u32> StrongCount{0};
        std::atomic<u32> WeakCount{0};
        std::atomic<u32> Generation{0};
        std::atomic<AssetState> State{AssetState::Unknown};
        std::mutex Mutex;

        void AddStrongRef() noexcept { StrongCount.fetch_add(1, std::memory_order_relaxed); }
        void ReleaseStrongRef() noexcept
        {
            if (StrongCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                // Last strong reference — asset can be unloaded.
                // Weak references keep the control block alive.
                State.store(AssetState::Unloading, std::memory_order_release);
            }
        }

        void AddWeakRef() noexcept { WeakCount.fetch_add(1, std::memory_order_relaxed); }
        void ReleaseWeakRef() noexcept
        {
            if (WeakCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                // Last weak reference — control block can be deleted.
            }
        }

        [[nodiscard]] u32 GetStrongCount() const noexcept
        { return StrongCount.load(std::memory_order_acquire); }

        [[nodiscard]] bool IsAlive() const noexcept
        { return GetStrongCount() > 0; }
    };

    // ========================================================================
    // AssetRef — strong reference (keeps asset alive).
    // ========================================================================

    /// @brief A strong reference to an asset.  While at least one
    ///        AssetRef exists, the asset's runtime data will not be
    ///        unloaded.
    class AssetRef
    {
    public:
        AssetRef() = default;
        AssetRef(std::nullptr_t) noexcept {}

        AssetRef(IAsset* asset, AssetControlBlock* controlBlock) noexcept
            : m_Asset(asset)
            , m_ControlBlock(controlBlock)
        {
            if (m_ControlBlock)
                m_ControlBlock->AddStrongRef();
        }

        AssetRef(const AssetRef& other) noexcept
            : m_Asset(other.m_Asset)
            , m_ControlBlock(other.m_ControlBlock)
        {
            if (m_ControlBlock)
                m_ControlBlock->AddStrongRef();
        }

        AssetRef(AssetRef&& other) noexcept
            : m_Asset(other.m_Asset)
            , m_ControlBlock(other.m_ControlBlock)
        {
            other.m_Asset = nullptr;
            other.m_ControlBlock = nullptr;
        }

        AssetRef& operator=(const AssetRef& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_Asset = other.m_Asset;
                m_ControlBlock = other.m_ControlBlock;
                if (m_ControlBlock)
                    m_ControlBlock->AddStrongRef();
            }
            return *this;
        }

        AssetRef& operator=(AssetRef&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_Asset = other.m_Asset;
                m_ControlBlock = other.m_ControlBlock;
                other.m_Asset = nullptr;
                other.m_ControlBlock = nullptr;
            }
            return *this;
        }

        ~AssetRef() { Release(); }

        [[nodiscard]] IAsset* Get() const noexcept { return m_Asset; }
        [[nodiscard]] IAsset* operator->() const noexcept { return m_Asset; }
        [[nodiscard]] IAsset& operator*() const noexcept { return *m_Asset; }

        [[nodiscard]] bool IsValid() const noexcept
        { return m_Asset != nullptr && m_ControlBlock != nullptr && m_ControlBlock->IsAlive(); }

        [[nodiscard]] bool IsNull() const noexcept { return m_Asset == nullptr; }

        void Reset() { Release(); m_Asset = nullptr; m_ControlBlock = nullptr; }

        [[nodiscard]] AssetHandle GetHandle() const noexcept;

    private:
        void Release()
        {
            if (m_ControlBlock)
                m_ControlBlock->ReleaseStrongRef();
        }

        IAsset*             m_Asset{nullptr};
        AssetControlBlock*  m_ControlBlock{nullptr};

        friend class AssetWeakRef;
    };

    // ========================================================================
    // AssetWeakRef — weak reference (does not keep asset alive).
    // ========================================================================

    /// @brief A weak reference to an asset.  Does not prevent the asset
    ///        from being unloaded.  Use Lock() to obtain a strong
    ///        reference if the asset is still alive.
    class AssetWeakRef
    {
    public:
        AssetWeakRef() = default;

        AssetWeakRef(const AssetRef& strong) noexcept
            : m_ControlBlock(strong.m_ControlBlock)
        {
            if (m_ControlBlock)
                m_ControlBlock->AddWeakRef();
        }

        AssetWeakRef(const AssetWeakRef& other) noexcept
            : m_ControlBlock(other.m_ControlBlock)
        {
            if (m_ControlBlock)
                m_ControlBlock->AddWeakRef();
        }

        AssetWeakRef(AssetWeakRef&& other) noexcept
            : m_ControlBlock(other.m_ControlBlock)
        {
            other.m_ControlBlock = nullptr;
        }

        AssetWeakRef& operator=(const AssetWeakRef& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_ControlBlock = other.m_ControlBlock;
                if (m_ControlBlock)
                    m_ControlBlock->AddWeakRef();
            }
            return *this;
        }

        AssetWeakRef& operator=(AssetWeakRef&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_ControlBlock = other.m_ControlBlock;
                other.m_ControlBlock = nullptr;
            }
            return *this;
        }

        ~AssetWeakRef() { Release(); }

        /// @brief Attempts to obtain a strong reference.  Returns a null
        ///        AssetRef if the asset has been unloaded.
        [[nodiscard]] AssetRef Lock() const noexcept
        {
            if (!m_ControlBlock || !m_ControlBlock->IsAlive())
                return AssetRef{};
            // The actual IAsset pointer is retrieved by the AssetManager.
            // This is a simplified version — in production, the weak ref
            // would store the AssetHandle and query the manager.
            return AssetRef{};
        }

        [[nodiscard]] bool IsExpired() const noexcept
        {
            return !m_ControlBlock || !m_ControlBlock->IsAlive();
        }

        [[nodiscard]] u32 UseCount() const noexcept
        {
            return m_ControlBlock ? m_ControlBlock->GetStrongCount() : 0;
        }

        void Reset() { Release(); m_ControlBlock = nullptr; }

    private:
        void Release()
        {
            if (m_ControlBlock)
                m_ControlBlock->ReleaseWeakRef();
        }

        AssetControlBlock* m_ControlBlock{nullptr};
    };

} // namespace engine::assets
