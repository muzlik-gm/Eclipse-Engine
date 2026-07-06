// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLDebugLayer.h
// KHR_debug integration for the OpenGL backend.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Validation/IGraphicsValidation.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

#include <mutex>
#include <unordered_map>
#include <vector>

namespace engine::opengl {

    using engine::core::u32;
    using engine::rhi::DebugCallback;
    using engine::rhi::DebugMessage;
    using engine::rhi::DebugMessageSeverity;
    using engine::rhi::DebugMessageCategory;
    using engine::rhi::IGraphicsValidation;

    // ========================================================================
    // OpenGLDebugLayer
    // ========================================================================

    /// @brief Implements IGraphicsValidation using OpenGL's KHR_debug
    ///        extension.  Routes driver / shader / pipeline messages to
    ///        registered callbacks and the engine logging system.
    class OpenGLDebugLayer final : public IGraphicsValidation
    {
    public:
        OpenGLDebugLayer();
        ~OpenGLDebugLayer() override;

        OpenGLDebugLayer(const OpenGLDebugLayer&)            = delete;
        OpenGLDebugLayer& operator=(const OpenGLDebugLayer&) = delete;

        /// @brief Initializes the debug layer.  Must be called after the
        ///        GL context is current and GLAD is loaded.
        void Initialize();

        /// @brief Shuts down the debug layer.
        void Shutdown();

        // -- IGraphicsValidation interface ------------------------------------

        [[nodiscard]] bool IsValidationEnabled() const noexcept override;
        [[nodiscard]] bool IsDebugOutputEnabled() const noexcept override;

        u32 RegisterCallback(DebugCallback callback) override;
        void RemoveCallback(u32 subscriptionId) override;

        void RouteMessage(const DebugMessage& message) override;

        void BeginDebugGroup(engine::rhi::ICommandBuffer* cmdBuffer,
                             std::string_view name) override;
        void EndDebugGroup(engine::rhi::ICommandBuffer* cmdBuffer) override;
        void InsertDebugMarker(engine::rhi::ICommandBuffer* cmdBuffer,
                               std::string_view name) override;

        void SetObjectName(engine::rhi::IGraphicsObject* object,
                           std::string_view name) override;

        [[nodiscard]] u32 GetMessageCount() const noexcept override;
        [[nodiscard]] u32 GetMessageCountAtOrAbove(
            DebugMessageSeverity severity) const noexcept override;

        // -- OpenGL-specific --------------------------------------------------

        /// @brief Sets the debug label of an OpenGL object.
        void SetObjectLabel(u32 identifier, u32 handle, std::string_view name);

        /// @brief Begins a debug group in the GL command stream.
        void PushDebugGroup(std::string_view name);

        /// @brief Ends the most recent debug group.
        void PopDebugGroup();

        /// @brief Inserts a single debug marker.
        void InsertMarker(std::string_view name);

    private:
        // -- GL debug callback (C function pointer) ---------------------------
        static void GLAPIENTRY DebugMessageCallback(
            u32 source, u32 type, u32 id, u32 severity,
            engine::core::i32 length, const char* message, const void* userParam);

        // -- Helpers ----------------------------------------------------------
        [[nodiscard]] static DebugMessageSeverity GLSeverityToEngine(u32 severity) noexcept;
        [[nodiscard]] static DebugMessageCategory GLSourceToCategory(u32 source) noexcept;

        // -- State ------------------------------------------------------------
        bool                                  m_Initialized{false};
        bool                                  m_ValidationEnabled{false};
        bool                                  m_DebugOutputEnabled{false};

        mutable std::mutex                    m_CallbackMutex;
        std::unordered_map<u32, DebugCallback> m_Callbacks;
        u32                                   m_NextCallbackId{1};

        mutable std::mutex                    m_StatsMutex;
        u32                                   m_TotalMessages{0};
        u32                                   m_ErrorMessages{0};
        u32                                   m_WarningMessages{0};
    };

} // namespace engine::opengl
