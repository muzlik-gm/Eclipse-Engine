// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLVertexArray.cpp
// ============================================================================
#include "OpenGLVertexArray.h"
#include "OpenGLBuffer.h"
#include "OpenGLTypes.h"
#include "OpenGLDebugLayer.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    using namespace engine::rhi;
    using engine::core::u32;
    using engine::core::usize;

    OpenGLVertexArray::OpenGLVertexArray(OpenGLDebugLayer* debugLayer)
        : m_DebugLayer(debugLayer)
    {
        glGenVertexArrays(1, &m_Handle);
        if (m_Handle == 0)
            ENGINE_LOG_ERROR("OpenGLVertexArray — glGenVertexArrays failed");
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        if (m_Handle != 0)
        {
            glDeleteVertexArrays(1, &m_Handle);
            m_Handle = 0;
        }
    }

    void OpenGLVertexArray::SetDebugName(std::string_view name)
    {
        m_Name = std::string(name);
        if (m_DebugLayer && m_Handle != 0)
            m_DebugLayer->SetObjectLabel(GL_VERTEX_ARRAY, m_Handle, m_Name);
    }

    void OpenGLVertexArray::Bind()
    {
        glBindVertexArray(m_Handle);
    }

    void OpenGLVertexArray::BindVertexBuffer(u32 binding, IVertexBuffer* buffer, usize offset)
    {
        if (!buffer || m_Handle == 0)
            return;

        auto* glVB = static_cast<OpenGLVertexBuffer*>(buffer);

        Bind();
        glBindVertexBuffer(binding, glVB->GetHandle(),
                           static_cast<GLintptr>(offset),
                           static_cast<GLsizei>(glVB->GetStride()));
        ++m_BoundVBCount;
    }

    void OpenGLVertexArray::BindIndexBuffer(IIndexBuffer* buffer, IndexType type, usize /*offset*/)
    {
        if (!buffer || m_Handle == 0)
            return;

        auto* glIB = static_cast<OpenGLIndexBuffer*>(buffer);

        Bind();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIB->GetHandle());
        m_IndexType = type;
    }

    void OpenGLVertexArray::SetVertexLayout(u32 binding, const VertexLayoutDescription& layout)
    {
        if (m_Handle == 0)
            return;

        Bind();

        const VertexBindingDescription* bindingDesc = nullptr;
        for (const auto& b : layout.Bindings)
        {
            if (b.Binding == binding) { bindingDesc = &b; break; }
        }
        if (!bindingDesc)
            return;

        glVertexBindingDivisor(binding, bindingDesc->Instanced ? 1 : 0);

        for (const auto& attr : layout.Attributes)
        {
            if (attr.Binding != binding)
                continue;

            glEnableVertexAttribArray(attr.Location);

            const GLint size = [f = attr.Format]() -> GLint {
                switch (f)
                {
                    case GraphicsFormat::R32_Float:    return 1;
                    case GraphicsFormat::RG32_Float:   return 2;
                    case GraphicsFormat::RGB32_Float:  return 3;
                    case GraphicsFormat::RGBA32_Float: return 4;
                    case GraphicsFormat::R8_UNorm:
                    case GraphicsFormat::R8_UInt:
                    case GraphicsFormat::R8_SInt:      return 1;
                    case GraphicsFormat::RG8_UNorm:
                    case GraphicsFormat::RG8_UInt:     return 2;
                    case GraphicsFormat::RGBA8_UNorm:
                    case GraphicsFormat::RGBA8_UInt:
                    case GraphicsFormat::RGBA8_SInt:
                    case GraphicsFormat::RGBA8_sRGB:   return 4;
                    default:                           return 4;
                }
            }();

            const GLenum type = ToGLType(attr.Format);
            const GLboolean normalized = attr.Normalized ? GL_TRUE : GL_FALSE;

            glVertexAttribFormat(attr.Location, size, type, normalized, attr.Offset);
            glVertexAttribBinding(attr.Location, attr.Binding);
        }
    }

} // namespace engine::opengl
