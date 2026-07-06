// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Validation/IGraphicsValidation.h
// Abstract interface for graphics validation layers, debug callbacks, and
// diagnostic message routing.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <functional>
#include <string>
#include <string_view>

namespace engine::rhi {

    using engine::core::u32;

    // ========================================================================
    // DebugMessageSeverity
    // ========================================================================
    enum class DebugMessageSeverity : u32
    {
        Verbose = 0,
        Info    = 1,
        Warning = 2,
        Error   = 3,
        Fatal   = 4
    };

    // ========================================================================
    // DebugMessageCategory
    // ========================================================================
    enum class DebugMessageCategory : u32
    {
        General           = 0,
        Initialization    = 1,
        Performance       = 2,
        Shader            = 3,
        Pipeline          = 4,
        Resource          = 5,
        Deprecated        = 6,
        UndefinedBehavior = 7,
        API               = 8,
        Other             = 9
    };

    // ========================================================================
    // DebugMessage
    // ========================================================================
    struct DebugMessage
    {
        DebugMessageSeverity Severity{DebugMessageSeverity::Info};
        DebugMessageCategory Category{DebugMessageCategory::General};
        u32                  MessageId{0};
        std::string          Source;       // backend identifier
        std::string          Text;         // human-readable message
        std::string          ObjectName;   // optional related object name
    };

    // ========================================================================
    // DebugCallback
    // ========================================================================
    using DebugCallback = std::function<void(const DebugMessage&)>;

    // ========================================================================
    // IGraphicsValidation
    // ========================================================================

    /// @brief Interface for validation layers and debug output.  Backends
    ///        implement this to forward driver / API messages to the
    ///        engine logging system.
    class IGraphicsValidation
    {
    public:
        virtual ~IGraphicsValidation() = default;

        /// @brief Returns true if validation layers are active.
        [[nodiscard]] virtual bool IsValidationEnabled() const noexcept = 0;

        /// @brief Returns true if debug output is active.
        [[nodiscard]] virtual bool IsDebugOutputEnabled() const noexcept = 0;

        /// @brief Registers a debug callback.
        /// @return An opaque subscription id that can be used with
        ///         RemoveCallback.
        virtual u32 RegisterCallback(DebugCallback callback) = 0;

        /// @brief Removes a previously registered callback.
        virtual void RemoveCallback(u32 subscriptionId) = 0;

        /// @brief Routes a message to all registered callbacks.
        ///        Called by backend implementations when they receive a
        ///        driver / API message.
        virtual void RouteMessage(const DebugMessage& message) = 0;

        /// @brief Begins a debug group on the given command buffer.
        virtual void BeginDebugGroup(class ICommandBuffer* cmdBuffer,
                                     std::string_view name) = 0;

        /// @brief Ends the most recently begun debug group.
        virtual void EndDebugGroup(class ICommandBuffer* cmdBuffer) = 0;

        /// @brief Inserts a debug marker on the given command buffer.
        virtual void InsertDebugMarker(class ICommandBuffer* cmdBuffer,
                                       std::string_view name) = 0;

        /// @brief Sets the debug name of a graphics object.
        virtual void SetObjectName(class IGraphicsObject* object,
                                   std::string_view name) = 0;

        /// @brief Returns the total number of messages received.
        [[nodiscard]] virtual u32 GetMessageCount() const noexcept = 0;

        /// @brief Returns the number of messages at or above the given
        ///        severity (used for error reporting).
        [[nodiscard]] virtual u32 GetMessageCountAtOrAbove(
            DebugMessageSeverity severity) const noexcept = 0;
    };

} // namespace engine::rhi
