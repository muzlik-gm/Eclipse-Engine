// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLBuffer.cpp
// ============================================================================
#include "OpenGLBuffer.h"
#include "OpenGLTypes.h"
#include "OpenGLDebugLayer.h"
#include "OpenGLStateCache.h"
#include "Engine/Core/Log.h"

namespace engine::opengl {

    using namespace engine::rhi;
    using engine::core::usize;

    // ========================================================================
    // OpenGLBuffer — base
    // ========================================================================

    OpenGLBuffer::OpenGLBuffer(const BufferDescription& desc, OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_Name(desc.Name)
        , m_DebugLayer(debugLayer)
    {
        m_Target = ToGLBufferTarget(desc.Usage);

        glGenBuffers(1, &m_Handle);
        if (m_Handle == 0)
        {
            ENGINE_LOG_ERROR("OpenGLBuffer — glGenBuffers failed");
            return;
        }

        // Allocate storage.
        glBindBuffer(m_Target, m_Handle);

        const u32 glUsage = ToGLBufferUsage(desc.Usage, desc.Immutable, desc.HostVisible);
        glBufferData(m_Target, static_cast<GLsizeiptr>(desc.Size), nullptr, glUsage);

        glBindBuffer(m_Target, 0);

        if (m_DebugLayer && !m_Name.empty())
        {
            m_DebugLayer->SetObjectLabel(GL_BUFFER, m_Handle, m_Name);
        }

        ENGINE_LOG_DEBUG("OpenGLBuffer — created '{}' (handle={}, size={}, target=0x{:X})",
                         m_Name, m_Handle, desc.Size, m_Target);
    }

    OpenGLBuffer::~OpenGLBuffer()
    {
        if (m_MappedPointer)
        {
            glBindBuffer(m_Target, m_Handle);
            glUnmapBuffer(m_Target);
            glBindBuffer(m_Target, 0);
            m_MappedPointer = nullptr;
        }

        if (m_Handle != 0)
        {
            glDeleteBuffers(1, &m_Handle);
            m_Handle = 0;
        }
    }

    void OpenGLBuffer::SetDebugName(std::string_view name)
    {
        m_Name = std::string(name);
        if (m_DebugLayer && m_Handle != 0)
        {
            m_DebugLayer->SetObjectLabel(GL_BUFFER, m_Handle, m_Name);
        }
    }

    void* OpenGLBuffer::Map(MapFlags flags, usize offset, usize size)
    {
        if (m_Handle == 0)
            return nullptr;

        if (m_MappedPointer)
        {
            ENGINE_LOG_WARN("OpenGLBuffer — buffer already mapped");
            return m_MappedPointer;
        }

        const usize mapSize = (size == 0) ? (m_Description.Size - offset) : size;

        glBindBuffer(m_Target, m_Handle);

        const u32 access = ToGLMapAccess(flags);

        // Use glMapBufferRange for partial / persistent mapping.
        void* ptr = glMapBufferRange(m_Target,
                                      static_cast<GLintptr>(offset),
                                      static_cast<GLsizeiptr>(mapSize),
                                      access);

        if (!ptr)
        {
            ENGINE_LOG_ERROR("OpenGLBuffer — glMapBufferRange failed");
            GLenum err = glGetError();
            ENGINE_LOG_ERROR("OpenGLBuffer — GL error: 0x{:X}", err);
        }
        else
        {
            m_MappedPointer = ptr;
            m_PersistentMapped = (flags & MapFlags::Persistent) != 0;
        }

        glBindBuffer(m_Target, 0);
        return ptr;
    }

    void OpenGLBuffer::Unmap()
    {
        if (!m_MappedPointer)
            return;

        glBindBuffer(m_Target, m_Handle);
        glUnmapBuffer(m_Target);
        glBindBuffer(m_Target, 0);
        m_MappedPointer = nullptr;
        m_PersistentMapped = false;
    }

    void OpenGLBuffer::FlushMappedRange(usize offset, usize size)
    {
        if (!m_MappedPointer)
            return;

        glBindBuffer(m_Target, m_Handle);
        glFlushMappedBufferRange(m_Target,
                                  static_cast<GLintptr>(offset),
                                  static_cast<GLsizeiptr>(size));
        glBindBuffer(m_Target, 0);
    }

    void OpenGLBuffer::InvalidateMappedRange(usize offset, usize size)
    {
        glBindBuffer(m_Target, m_Handle);
        glInvalidateBufferSubData(m_Handle,
                                   static_cast<GLintptr>(offset),
                                   static_cast<GLsizeiptr>(size));
        glBindBuffer(m_Target, 0);
    }

    void OpenGLBuffer::UpdateData(const void* data, usize size, usize offset)
    {
        if (m_Handle == 0 || !data || size == 0)
            return;

        glBindBuffer(m_Target, m_Handle);

        if (offset == 0 && size == m_Description.Size)
        {
            // Full update — re-specify storage.
            const u32 glUsage = ToGLBufferUsage(m_Description.Usage,
                                                 m_Description.Immutable,
                                                 m_Description.HostVisible);
            glBufferData(m_Target, static_cast<GLsizeiptr>(size), data, glUsage);
        }
        else
        {
            // Partial update.
            glBufferSubData(m_Target,
                            static_cast<GLintptr>(offset),
                            static_cast<GLsizeiptr>(size),
                            data);
        }

        glBindBuffer(m_Target, 0);
    }

    void OpenGLBuffer::Bind(OpenGLStateCache& stateCache)
    {
        if (stateCache.SetBuffer(m_Target, m_Handle))
        {
            glBindBuffer(m_Target, m_Handle);
        }
    }

    // ========================================================================
    // OpenGLVertexBuffer
    // ========================================================================

    OpenGLVertexBuffer::OpenGLVertexBuffer(const BufferDescription& desc,
                                             OpenGLDebugLayer* debugLayer)
        : OpenGLBuffer(desc, debugLayer)
    {
        // m_Target already set to GL_ARRAY_BUFFER by base constructor.
    }

    // ========================================================================
    // OpenGLIndexBuffer
    // ========================================================================

    OpenGLIndexBuffer::OpenGLIndexBuffer(const BufferDescription& desc,
                                           OpenGLDebugLayer* debugLayer)
        : OpenGLBuffer(desc, debugLayer)
    {
        m_Target = GL_ELEMENT_ARRAY_BUFFER;
    }

    // ========================================================================
    // OpenGLUniformBuffer
    // ========================================================================

    OpenGLUniformBuffer::OpenGLUniformBuffer(const BufferDescription& desc,
                                               OpenGLDebugLayer* debugLayer)
        : OpenGLBuffer(desc, debugLayer)
    {
        m_Target = GL_UNIFORM_BUFFER;
    }

    void OpenGLUniformBuffer::Bind(OpenGLStateCache& stateCache)
    {
        // UBOs use indexed binding.
        glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_Handle);
    }

    // ========================================================================
    // OpenGLStorageBuffer
    // ========================================================================

    OpenGLStorageBuffer::OpenGLStorageBuffer(const BufferDescription& desc,
                                               OpenGLDebugLayer* debugLayer)
        : OpenGLBuffer(desc, debugLayer)
    {
        m_Target = GL_SHADER_STORAGE_BUFFER;
    }

    void OpenGLStorageBuffer::Bind(OpenGLStateCache& stateCache)
    {
        // SSBOs use indexed binding — assume binding point 0 for now.
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_Handle);
    }

} // namespace engine::opengl
