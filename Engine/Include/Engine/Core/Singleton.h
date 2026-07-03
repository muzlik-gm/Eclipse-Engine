// ============================================================================
// File: Engine/Include/Engine/Core/Singleton.h
// ============================================================================

#pragma once

/**
 * @file Singleton.h
 * @brief CRTP singleton base class.
 *
 * Derive privately from Singleton<T> and friend it to obtain Meyers'
 * singleton semantics with a single Instance() accessor.  Construction
 * happens on first access and destruction at static deinitialisation.
 */

#include "Engine/Core/Types.h"

namespace engine::core {

/// CRTP singleton base. Derive privately: class MySingleton : public Singleton<MySingleton> { friend class Singleton<MySingleton>; ... };
template <typename T>
class Singleton
{
public:
    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&)                 = delete;
    Singleton& operator=(Singleton&&)      = delete;

    /// Returns the single instance, created on first access.
    [[nodiscard]] static T& Instance()
    {
        static T instance;
        return instance;
    }

protected:
    Singleton()  = default;
    ~Singleton() = default;
};

} // namespace engine::core