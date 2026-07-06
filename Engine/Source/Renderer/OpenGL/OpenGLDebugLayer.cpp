// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLDebugLayer.cpp
// ============================================================================
#include "OpenGLDebugLayer.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"

#include "Engine/Core/Log.h"

namespace engine::opengl {

    using engine::rhi::DebugMessage;

    // ========================================================================
    // Construction / destruction
    // ========================================================================

    OpenGLDebugLayer::OpenGLDebugLayer() = default;

    OpenGLDebugLayer::~OpenGLDebugLayer()
    {
        Shutdown();
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    void OpenGLDebugLayer::Initialize()
    {
        if (m_Initialized)
            return;

        // Check if KHR_debug is available (OpenGL 4.3+).
        if (glDebugMessageCallback)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(DebugMessageCallback, this);

            // Enable all messages by default.
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
                                  0, nullptr, GL_TRUE);

            m_DebugOutputEnabled = true;
            m_ValidationEnabled = true;

            ENGINE_LOG_INFO("OpenGLDebugLayer — KHR_debug initialized");
        }
        else
        {
            ENGINE_LOG_WARN("OpenGLDebugLayer — KHR_debug not available, debug output disabled");
        }

        m_Initialized = true;
    }

    void OpenGLDebugLayer::Shutdown()
    {
        if (!m_Initialized)
            return;

        if (glDebugMessageCallback && m_DebugOutputEnabled)
        {
            glDisable(GL_DEBUG_OUTPUT);
        }

        std::lock_guard lock(m_CallbackMutex);
        m_Callbacks.clear();

        m_Initialized = false;
        m_DebugOutputEnabled = false;
        m_ValidationEnabled = false;
    }

    // ========================================================================
    // IGraphicsValidation interface
    // ========================================================================

    bool OpenGLDebugLayer::IsValidationEnabled() const noexcept
    {
        return m_ValidationEnabled;
    }

    bool OpenGLDebugLayer::IsDebugOutputEnabled() const noexcept
    {
        return m_DebugOutputEnabled;
    }

    u32 OpenGLDebugLayer::RegisterCallback(DebugCallback callback)
    {
        std::lock_guard lock(m_CallbackMutex);
        u32 id = m_NextCallbackId++;
        m_Callbacks[id] = std::move(callback);
        return id;
    }

    void OpenGLDebugLayer::RemoveCallback(u32 subscriptionId)
    {
        std::lock_guard lock(m_CallbackMutex);
        m_Callbacks.erase(subscriptionId);
    }

    void OpenGLDebugLayer::RouteMessage(const DebugMessage& message)
    {
        {
            std::lock_guard lock(m_StatsMutex);
            ++m_TotalMessages;
            if (message.Severity >= DebugMessageSeverity::Error)
                ++m_ErrorMessages;
            else if (message.Severity >= DebugMessageSeverity::Warning)
                ++m_WarningMessages;
        }

        // Route to the engine log first.
        switch (message.Severity)
        {
            case DebugMessageSeverity::Verbose:
                ENGINE_LOG_TRACE("[GL] {} (id={}): {}", message.Source, message.MessageId, message.Text);
                break;
            case DebugMessageSeverity::Info:
                ENGINE_LOG_INFO("[GL] {} (id={}): {}", message.Source, message.MessageId, message.Text);
                break;
            case DebugMessageSeverity::Warning:
                ENGINE_LOG_WARN("[GL] {} (id={}): {}", message.Source, message.MessageId, message.Text);
                break;
            case DebugMessageSeverity::Error:
            case DebugMessageSeverity::Fatal:
                ENGINE_LOG_ERROR("[GL] {} (id={}): {}", message.Source, message.MessageId, message.Text);
                break;
        }

        // Then route to registered callbacks.
        std::lock_guard lock(m_CallbackMutex);
        for (auto& [id, callback] : m_Callbacks)
        {
            callback(message);
        }
    }

    void OpenGLDebugLayer::BeginDebugGroup(engine::rhi::ICommandBuffer* /*cmdBuffer*/,
                                            std::string_view name)
    {
        PushDebugGroup(name);
    }

    void OpenGLDebugLayer::EndDebugGroup(engine::rhi::ICommandBuffer* /*cmdBuffer*/)
    {
        PopDebugGroup();
    }

    void OpenGLDebugLayer::InsertDebugMarker(engine::rhi::ICommandBuffer* /*cmdBuffer*/,
                                              std::string_view name)
    {
        InsertMarker(name);
    }

    void OpenGLDebugLayer::SetObjectName(engine::rhi::IGraphicsObject* /*object*/,
                                          std::string_view name)
    {
        // The object name is set on the concrete GL object directly via
        // SetObjectLabel.  This method exists for interface completeness.
        (void)name;
    }

    u32 OpenGLDebugLayer::GetMessageCount() const noexcept
    {
        return m_TotalMessages;
    }

    u32 OpenGLDebugLayer::GetMessageCountAtOrAbove(
        DebugMessageSeverity severity) const noexcept
    {
        if (severity <= DebugMessageSeverity::Warning)
            return m_ErrorMessages + m_WarningMessages;
        return m_ErrorMessages;
    }

    // ========================================================================
    // OpenGL-specific
    // ========================================================================

    void OpenGLDebugLayer::SetObjectLabel(u32 identifier, u32 handle,
                                           std::string_view name)
    {
        if (m_Initialized && glObjectLabel)
        {
            glObjectLabel(identifier, handle,
                          static_cast<GLsizei>(name.size()),
                          name.data());
        }
    }

    void OpenGLDebugLayer::PushDebugGroup(std::string_view name)
    {
        if (m_Initialized && glPushDebugGroup)
        {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0,
                             static_cast<GLsizei>(name.size()),
                             name.data());
        }
    }

    void OpenGLDebugLayer::PopDebugGroup()
    {
        if (m_Initialized && glPopDebugGroup)
        {
            glPopDebugGroup();
        }
    }

    void OpenGLDebugLayer::InsertMarker(std::string_view name)
    {
        if (m_Initialized && glDebugMessageInsert)
        {
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                                  GL_DEBUG_TYPE_MARKER,
                                  0,
                                  GL_DEBUG_SEVERITY_NOTIFICATION,
                                  static_cast<GLsizei>(name.size()),
                                  name.data());
        }
    }

    // ========================================================================
    // GL debug callback (static)
    // ========================================================================

    void GLAPIENTRY OpenGLDebugLayer::DebugMessageCallback(
        u32 source, u32 type, u32 id, u32 severity,
        engine::core::i32 length, const char* message, const void* userParam)
    {
        auto* self = static_cast<OpenGLDebugLayer*>(const_cast<void*>(userParam));
        if (!self || !message)
            return;

        DebugMessage msg;
        msg.Severity = GLSeverityToEngine(severity);
        msg.Category = GLSourceToCategory(source);
        msg.MessageId = id;
        msg.Source = "OpenGL";
        msg.Text = std::string(message, (length > 0) ? static_cast<engine::core::usize>(length) : std::strlen(message));

        self->RouteMessage(msg);
    }

    DebugMessageSeverity OpenGLDebugLayer::GLSeverityToEngine(u32 severity) noexcept
    {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         return DebugMessageSeverity::Error;
            case GL_DEBUG_SEVERITY_MEDIUM:       return DebugMessageSeverity::Warning;
            case GL_DEBUG_SEVERITY_LOW:          return DebugMessageSeverity::Info;
            case GL_DEBUG_SEVERITY_NOTIFICATION: return DebugMessageSeverity::Verbose;
            default:                             return DebugMessageSeverity::Info;
        }
    }

    DebugMessageCategory OpenGLDebugLayer::GLSourceToCategory(u32 source) noexcept
    {
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:             return DebugMessageCategory::API;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return DebugMessageCategory::Shader;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     return DebugMessageCategory::Other;
            case GL_DEBUG_SOURCE_APPLICATION:     return DebugMessageCategory::General;
            case GL_DEBUG_SOURCE_OTHER:           return DebugMessageCategory::General;
            default:                              return DebugMessageCategory::General;
        }
    }

} // namespace engine::opengl
