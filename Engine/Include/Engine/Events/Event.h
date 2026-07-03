// ============================================================================
// File: Engine/Include/Engine/Events/Event.h
// ============================================================================
#pragma once

#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Types.h"

#include <functional>
#include <string>
#include <string_view>

namespace engine::events {

    using engine::core::u32;

    // ========================================================================
    // EventType
    // ========================================================================
    enum class EventType : u32
    {
        None                  = 0,
        WindowClose           = 1,
        WindowResize          = 2,
        WindowFocus           = 3,
        WindowLostFocus       = 4,
        WindowMoved           = 5,
        KeyPressed            = 6,
        KeyReleased           = 7,
        KeyTyped              = 8,
        MouseMoved            = 9,
        MouseScrolled         = 10,
        MouseButtonPressed    = 11,
        MouseButtonReleased   = 12
    };

    // ========================================================================
    // EventCategory (bit-flag enum)
    // ========================================================================
    enum class EventCategory : u32
    {
        None        = 0,
        Application = (1u << 0),
        Input       = (1u << 1),
        Keyboard    = (1u << 2),
        Mouse       = (1u << 3),
        MouseButton = (1u << 4)
    };

    ENGINE_ENUM_FLAGS(EventCategory);

    // ========================================================================
    // Event – base class for all engine events
    // ========================================================================
    class Event
    {
    public:
        virtual ~Event() = default;

        [[nodiscard]] virtual constexpr std::string_view GetName() const noexcept = 0;
        [[nodiscard]] virtual constexpr EventType       GetEventType() const noexcept = 0;
        [[nodiscard]] virtual constexpr EventCategory   GetCategoryFlags() const noexcept = 0;

        [[nodiscard]] constexpr bool IsInCategory(EventCategory category) const noexcept
        {
            return static_cast<u32>(GetCategoryFlags()) & static_cast<u32>(category);
        }

        [[nodiscard]] constexpr bool IsHandled() const noexcept { return m_Handled; }
        constexpr void SetHandled(bool handled) noexcept { m_Handled = handled; }

    protected:
        bool m_Handled = false;
    };

} // namespace engine::events

// ---------------------------------------------------------------------------
// Optional debug string helper – only compiled when ENABLE_DEBUG_STRING is 1.
// ---------------------------------------------------------------------------
#if defined(ENABLE_DEBUG_STRING) && ENABLE_DEBUG_STRING
    #define ENGINE_EVENT_DEBUG_STRING(EventClass)                                    \
        [[nodiscard]] std::string ToString() const override                          \
        {                                                                            \
            return std::string(GetName()) + ": " + EventClass::DebugString();        \
        }
#else
    #define ENGINE_EVENT_DEBUG_STRING(EventClass)
#endif