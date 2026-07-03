#pragma once

/**
 * @file Thread.h
 * @brief Engine threading primitives: named thread wrapper, RAII scoped lock,
 *        and shared mutex.
 *
 * Thread wraps std::thread with RAII semantics (auto-joins in the destructor)
 * and an associated debug name.  ScopedLock is a lightweight non-copyable RAII
 * lock guard that works with any lockable type.  SharedMutex wraps
 * std::shared_mutex for reader-writer locking.
 */

#include "Engine/Core/Types.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>

namespace engine::threading
{

    using engine::core::i32;
    using engine::core::i64;

    // ========================================================================
    //  Thread
    // ========================================================================

    /// RAII wrapper around std::thread with an optional debug name.
    /// The destructor automatically joins the thread if it has not yet been
    /// joined or detached, making resource leaks significantly harder.
    class Thread
    {
    public:
        /// Constructs and launches a new thread that executes @p task.
        /// @param name  A human-readable name used for debugging / logging.
        /// @param task  The callable to execute on the new thread.
        Thread(std::string_view name, std::function<void()> task);

        /// If the thread is still joinable it is joined automatically.
        ~Thread();

        Thread(const Thread&)            = delete;
        Thread& operator=(const Thread&) = delete;

        Thread(Thread&& other) noexcept;
        Thread& operator=(Thread&& other) noexcept;

        /// Blocks until the thread finishes execution.
        /// Throws std::system_error if the thread has already been joined.
        void Join();

        /// Returns true if the thread is joinable (not yet joined /
        /// detached).
        [[nodiscard]] bool Joinable() const;

        /// Detaches the thread so it runs independently.
        void Detach();

        /// Returns the unique identifier of the underlying std::thread.
        [[nodiscard]] std::thread::id GetId() const;

        /// Returns the human-readable name assigned at construction.
        [[nodiscard]] std::string GetName() const;

        /// Returns the platform-native thread handle.
        /// Non-const because the underlying std::thread::native_handle()
        /// is not const-qualified.
        [[nodiscard]] std::thread::native_handle_type NativeHandle();

    private:
        std::thread m_thread;
        std::string m_name;
        bool        m_joined = false;
    };

    // ========================================================================
    //  ScopedLock
    // ========================================================================

    /// Lightweight, non-copyable, non-movable RAII lock guard.
    /// Locks the given mutex on construction and unlocks on destruction.
    /// @tparam Mutex Any type satisfying the BasicLockable requirement
    ///               (provides lock() / unlock()).
    template <typename Mutex>
    class ScopedLock
    {
    public:
        /// Acquires the lock on @p mutex.
        explicit ScopedLock(Mutex& mutex)
            : m_mutex(mutex)
        {
            m_mutex.lock();
        }

        /// Releases the lock.
        ~ScopedLock()
        {
            m_mutex.unlock();
        }

        ScopedLock(const ScopedLock&)            = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;
        ScopedLock(ScopedLock&&)                 = delete;
        ScopedLock& operator=(ScopedLock&&)      = delete;

        friend class ConditionVariable;

    private:
        Mutex& m_mutex;
    };

    // ========================================================================
    //  SharedMutex
    // ========================================================================

    /// Thin wrapper around std::shared_mutex providing named methods for
    /// exclusive and shared (reader-writer) locking.
    class SharedMutex
    {
    public:
        SharedMutex()  = default;
        ~SharedMutex() = default;

        SharedMutex(const SharedMutex&)            = delete;
        SharedMutex& operator=(const SharedMutex&) = delete;

        // -- Exclusive (write) locking -----------------------------------

        /// Blocks until an exclusive lock is acquired.
        void Lock();

        /// Releases an exclusive lock.
        void Unlock();

        /// Tries to acquire an exclusive lock without blocking.
        /// Returns true if the lock was acquired.
        [[nodiscard]] bool TryLock();

        // -- Shared (read) locking ---------------------------------------

        /// Blocks until a shared (reader) lock is acquired.
        void LockShared();

        /// Releases a shared lock.
        void UnlockShared();

        /// Tries to acquire a shared lock without blocking.
        /// Returns true if the lock was acquired.
        [[nodiscard]] bool TryLockShared();

    private:
        std::shared_mutex m_mutex;
    };

    // ========================================================================
    //  ConditionVariable
    // ========================================================================

    /// Thin wrapper around std::condition_variable.
    class ConditionVariable
    {
    public:
        ConditionVariable()  = default;
        ~ConditionVariable() = default;

        ConditionVariable(const ConditionVariable&)            = delete;
        ConditionVariable& operator=(const ConditionVariable&) = delete;

        /// Blocks until notified, or spurious wakeup.
        void Wait(ScopedLock<std::mutex>& lock);

        /// Blocks until notified, predicate returns true, or timeout.
        /// Returns true if predicate returned true, false on timeout.
        template <typename Predicate>
        bool WaitFor(ScopedLock<std::mutex>& lock, Predicate pred, i64 timeoutMs);

        /// Notifies one waiting thread.
        void NotifyOne();

        /// Notifies all waiting threads.
        void NotifyAll();

    private:
        std::condition_variable m_condVar;
    };

    template <typename Predicate>
    bool ConditionVariable::WaitFor(ScopedLock<std::mutex>& lock, Predicate pred, i64 timeoutMs)
    {
        // ScopedLock holds a raw mutex reference; condition_variable requires unique_lock.
        // Temporarily transfer ownership to a unique_lock for the wait.
        lock.m_mutex.unlock();
        std::unique_lock<std::mutex> uLock(lock.m_mutex);
        bool result = m_condVar.wait_for(uLock, std::chrono::milliseconds(timeoutMs), pred);
        uLock.release(); // hand ownership back so ScopedLock's dtor can unlock
        return result;
    }

} // namespace engine::threading