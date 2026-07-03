#include "Engine/Threading/Thread.h"

#include <stdexcept>

namespace engine::threading
{

    // ========================================================================
    //  Thread
    // ========================================================================

    Thread::Thread(std::string_view name, std::function<void()> task)
        : m_name(name)
        , m_joined(false)
    {
        m_thread = std::thread([this, task = std::move(task)]()
        {
            // On platforms that support thread naming, set it here.
#if ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
            if (!m_name.empty())
            {
                // pthread_setname_np has different signatures on Linux vs macOS.
                // Linux: takes (pthread_t, name, len) — name length limited to 15 chars.
                // macOS:  takes (pthread_t, name)       — name limited to MAXTHREADNAMES (64).
#if ENGINE_PLATFORM_MACOS
                ::pthread_setname_np(m_name.c_str());
#else
                // Truncate to 15 characters (null-terminator is included in the limit
                // on Linux).
                std::string truncated = m_name.substr(0, 15);
                ::pthread_setname_np(m_thread.native_handle(), truncated.c_str());
#endif
            }
#endif

            task();
        });
    }

    Thread::~Thread()
    {
        if (Joinable() && !m_joined)
        {
            try
            {
                Join();
            }
            catch (const std::exception&)
            {
                // Suppress exceptions in the destructor.  If the thread is
                // detached or already joined, Join() simply returns.
            }
        }
    }

    Thread::Thread(Thread&& other) noexcept
        : m_thread(std::move(other.m_thread))
        , m_name(std::move(other.m_name))
        , m_joined(other.m_joined)
    {
        other.m_joined = true; // Prevent the moved-from object from joining.
    }

    Thread& Thread::operator=(Thread&& other) noexcept
    {
        if (this != &other)
        {
            if (Joinable() && !m_joined)
            {
                m_thread.join();
            }

            m_thread = std::move(other.m_thread);
            m_name   = std::move(other.m_name);
            m_joined = other.m_joined;

            other.m_joined = true;
        }
        return *this;
    }

    void Thread::Join()
    {
        if (m_joined)
        {
            throw std::system_error(
                std::make_error_code(std::errc::invalid_argument),
                "Thread::Join — thread has already been joined or moved from");
        }

        if (m_thread.joinable())
        {
            m_thread.join();
            m_joined = true;
        }
    }

    bool Thread::Joinable() const
    {
        return m_thread.joinable();
    }

    void Thread::Detach()
    {
        if (m_thread.joinable())
        {
            m_thread.detach();
            m_joined = true; // Prevent double-join/detach.
        }
    }

    std::thread::id Thread::GetId() const
    {
        return m_thread.get_id();
    }

    std::string Thread::GetName() const
    {
        return m_name;
    }

    std::thread::native_handle_type Thread::NativeHandle()
    {
        return m_thread.native_handle();
    }

    // ========================================================================
    //  SharedMutex
    // ========================================================================

    void SharedMutex::Lock()
    {
        m_mutex.lock();
    }

    void SharedMutex::Unlock()
    {
        m_mutex.unlock();
    }

    bool SharedMutex::TryLock()
    {
        return m_mutex.try_lock();
    }

    void SharedMutex::LockShared()
    {
        m_mutex.lock_shared();
    }

    void SharedMutex::UnlockShared()
    {
        m_mutex.unlock_shared();
    }

    bool SharedMutex::TryLockShared()
    {
        return m_mutex.try_lock_shared();
    }

} // namespace engine::threading