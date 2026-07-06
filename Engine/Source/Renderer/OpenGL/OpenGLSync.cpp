// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLSync.cpp
// ============================================================================
#include "OpenGLSync.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    using engine::core::u64;

    // ========================================================================
    // OpenGLFence
    // ========================================================================

    OpenGLFence::OpenGLFence(bool signaled, OpenGLDebugLayer* /*debugLayer*/)
        : m_Signaled(signaled)
    {
        if (!signaled)
        {
            m_Sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }
    }

    OpenGLFence::~OpenGLFence()
    {
        if (m_Sync)
        {
            glDeleteSync(m_Sync);
            m_Sync = nullptr;
        }
    }

    void OpenGLFence::Reset()
    {
        if (m_Sync)
        {
            glDeleteSync(m_Sync);
            m_Sync = nullptr;
        }
        m_Sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        m_Signaled = false;
    }

    bool OpenGLFence::IsSignaled() const
    {
        if (!m_Sync)
            return m_Signaled;

        GLint value = 0;
        GLsizei len = 0;
        glGetSynciv(m_Sync, GL_SYNC_STATUS, sizeof(GLint), &len, &value);
        return value == GL_SIGNALED;
    }

    void OpenGLFence::Wait()
    {
        if (!m_Sync)
            return;

        GLenum result = glClientWaitSync(m_Sync,
                                          GL_SYNC_FLUSH_COMMANDS_BIT,
                                          GL_TIMEOUT_IGNORED);
        if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
        {
            m_Signaled = true;
        }
    }

    bool OpenGLFence::WaitForNanoseconds(u64 timeoutNs)
    {
        if (!m_Sync)
            return m_Signaled;

        GLenum result = glClientWaitSync(m_Sync,
                                          GL_SYNC_FLUSH_COMMANDS_BIT,
                                          static_cast<GLuint64>(timeoutNs));
        if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
        {
            m_Signaled = true;
            return true;
        }
        return false;
    }

    // ========================================================================
    // OpenGLSemaphore — emulated (OpenGL has no semaphores)
    // ========================================================================

    OpenGLSemaphore::OpenGLSemaphore(u64 initialValue, OpenGLDebugLayer* /*debugLayer*/)
        : m_Value(initialValue)
    {
    }

    OpenGLSemaphore::~OpenGLSemaphore() = default;

    // ========================================================================
    // OpenGLDescriptorSet
    // ========================================================================

    void OpenGLDescriptorSet::BindSampler(u32 /*binding*/, ISampler* /*sampler*/)
    {
        // OpenGL binds samplers directly via glBindSampler in the command buffer.
    }

    void OpenGLDescriptorSet::BindTexture(u32 /*binding*/, ITexture* /*texture*/)
    {
        // OpenGL binds textures directly via glActiveTexture + glBindTexture.
    }

    void OpenGLDescriptorSet::BindUniformBuffer(u32 /*binding*/, IUniformBuffer* /*buffer*/)
    {
        // OpenGL binds UBOs directly via glBindBufferBase in the command buffer.
    }

    void OpenGLDescriptorSet::BindStorageBuffer(u32 /*binding*/, IStorageBuffer* /*buffer*/)
    {
        // OpenGL binds SSBOs directly via glBindBufferBase in the command buffer.
    }

    // ========================================================================
    // OpenGLDescriptorPool
    // ========================================================================

    engine::rhi::IDescriptorSet* OpenGLDescriptorPool::Allocate(IDescriptorLayout* layout)
    {
        if (m_Allocated >= m_MaxSets)
        {
            ENGINE_LOG_WARN("OpenGLDescriptorPool — pool exhausted (max={})", m_MaxSets);
            return nullptr;
        }

        auto* glLayout = static_cast<OpenGLDescriptorLayout*>(layout);
        auto set = std::make_unique<OpenGLDescriptorSet>(glLayout);
        auto* raw = set.get();
        m_Sets.push_back(std::move(set));
        ++m_Allocated;
        return raw;
    }

    void OpenGLDescriptorPool::Reset()
    {
        m_Sets.clear();
        m_Allocated = 0;
    }

} // namespace engine::opengl
