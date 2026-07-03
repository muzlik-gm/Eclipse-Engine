// ============================================================================
// File: Engine/Include/Engine/Platform/Cursor.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <memory>

namespace engine::platform {

    using engine::core::u32;
    using engine::core::f64;

    // ========================================================================
    // CursorShape
    // ========================================================================
    enum class CursorShape
    {
        Arrow     = 0,
        IBeam     = 1,
        Crosshair = 2,
        Hand      = 3,
        HResize   = 4,
        VResize   = 5,
        ResizeAll = 6,
        NoCursor  = 7
    };

    // ========================================================================
    // ICursor – pure-virtual cursor interface
    // ========================================================================
    class ICursor : private engine::core::NonCopyable
    {
    public:
        virtual ~ICursor() = default;

        virtual void SetShape(CursorShape shape) = 0;
        [[nodiscard]] virtual CursorShape GetShape() const = 0;

        [[nodiscard]] virtual bool IsVisible() const = 0;
        virtual void SetVisible(bool visible) = 0;

        virtual void SetPosition(f64 x, f64 y) = 0;
        virtual void GetPosition(f64& outX, f64& outY) const = 0;

        virtual void EnableLockMode() = 0;
        virtual void DisableLockMode() = 0;
        [[nodiscard]] virtual bool IsLocked() const = 0;

        // --------------------------------------------------------------------
        // Factory – creates a cursor bound to the given native window handle.
        // --------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<ICursor> Create(void* windowNativeHandle);
    };

} // namespace engine::platform