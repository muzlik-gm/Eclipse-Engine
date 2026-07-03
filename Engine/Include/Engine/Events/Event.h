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
    using engine::core::i32;
    using engine::core::f32;

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
        MouseButtonReleased   = 12,
        WindowMinimized       = 13,
        WindowMaximized       = 14,
        WindowRestored        = 15,
        WindowRefresh         = 16,
        FramebufferResized    = 17,
        DPIChanged            = 18
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
        MouseButton = (1u << 4),
        Window      = (1u << 5)
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

    // ========================================================================
    // Window Events
    // ========================================================================

    class WindowCreatedEvent final : public Event
    {
    public:
        explicit WindowCreatedEvent(u32 width, u32 height)
            : m_Width(width), m_Height(height) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowCreated"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window | EventCategory::Application; }

        [[nodiscard]] constexpr u32 GetWidth() const noexcept { return m_Width; }
        [[nodiscard]] constexpr u32 GetHeight() const noexcept { return m_Height; }

#if defined(ENABLE_DEBUG_STRING) && ENABLE_DEBUG_STRING
        [[nodiscard]] std::string ToString() const
        {
            return std::string(GetName()) + ": " + std::to_string(m_Width) + "x" + std::to_string(m_Height);
        }
#endif

    private:
        u32 m_Width;
        u32 m_Height;
    };

    class WindowCloseEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowClose"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowClose; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }
    };

    class WindowResizeEvent final : public Event
    {
    public:
        explicit WindowResizeEvent(u32 width, u32 height)
            : m_Width(width), m_Height(height) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowResize"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowResize; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }

        [[nodiscard]] constexpr u32 GetWidth() const noexcept { return m_Width; }
        [[nodiscard]] constexpr u32 GetHeight() const noexcept { return m_Height; }

#if defined(ENABLE_DEBUG_STRING) && ENABLE_DEBUG_STRING
        [[nodiscard]] std::string ToString() const
        {
            return std::string(GetName()) + ": " + std::to_string(m_Width) + "x" + std::to_string(m_Height);
        }
#endif

    private:
        u32 m_Width;
        u32 m_Height;
    };

    class WindowFocusEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowFocus"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowFocus; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }
    };

    class WindowLostFocusEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowLostFocus"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowLostFocus; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }
    };

    class WindowMovedEvent final : public Event
    {
    public:
        explicit WindowMovedEvent(i32 x, i32 y)
            : m_X(x), m_Y(y) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowMoved"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowMoved; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }

        [[nodiscard]] constexpr i32 GetX() const noexcept { return m_X; }
        [[nodiscard]] constexpr i32 GetY() const noexcept { return m_Y; }

#if defined(ENABLE_DEBUG_STRING) && ENABLE_DEBUG_STRING
        [[nodiscard]] std::string ToString() const
        {
            return std::string(GetName()) + ": (" + std::to_string(m_X) + ", " + std::to_string(m_Y) + ")";
        }
#endif

    private:
        i32 m_X;
        i32 m_Y;
    };

    class WindowMinimizedEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowMinimized"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowMinimized; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }
    };

    class WindowMaximizedEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowMaximized"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowMaximized; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }
    };

    class WindowRestoredEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowRestored"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowRestored; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }
    };

    class WindowRefreshEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WindowRefresh"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::WindowRefresh; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }
    };

    class FramebufferResizeEvent final : public Event
    {
    public:
        explicit FramebufferResizeEvent(u32 width, u32 height)
            : m_Width(width), m_Height(height) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "FramebufferResize"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::FramebufferResized; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }

        [[nodiscard]] constexpr u32 GetWidth() const noexcept { return m_Width; }
        [[nodiscard]] constexpr u32 GetHeight() const noexcept { return m_Height; }

#if defined(ENABLE_DEBUG_STRING) && ENABLE_DEBUG_STRING
        [[nodiscard]] std::string ToString() const
        {
            return std::string(GetName()) + ": " + std::to_string(m_Width) + "x" + std::to_string(m_Height);
        }
#endif

    private:
        u32 m_Width;
        u32 m_Height;
    };

    class DPIChangedEvent final : public Event
    {
    public:
        explicit DPIChangedEvent(f32 scaleX, f32 scaleY)
            : m_ScaleX(scaleX), m_ScaleY(scaleY) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "DPIChanged"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::DPIChanged; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Window; }

        [[nodiscard]] constexpr f32 GetScaleX() const noexcept { return m_ScaleX; }
        [[nodiscard]] constexpr f32 GetScaleY() const noexcept { return m_ScaleY; }

#if defined(ENABLE_DEBUG_STRING) && ENABLE_DEBUG_STRING
        [[nodiscard]] std::string ToString() const
        {
            return std::string(GetName()) + ": scaleX=" + std::to_string(m_ScaleX)
                 + ", scaleY=" + std::to_string(m_ScaleY);
        }
#endif

    private:
        f32 m_ScaleX;
        f32 m_ScaleY;
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