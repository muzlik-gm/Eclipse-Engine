// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLVertexArray.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"

#include <glad/glad.h>
#include <string>
#include <unordered_map>

namespace engine::opengl {

    class OpenGLDebugLayer;

    class OpenGLVertexArray final : public engine::rhi::IVertexArray
    {
    public:
        OpenGLVertexArray(OpenGLDebugLayer* debugLayer);
        ~OpenGLVertexArray() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override;
        engine::core::u64 GetNativeHandle() const noexcept override { return m_Handle; }
        bool IsValid() const noexcept override { return m_Handle != 0; }

        void BindVertexBuffer(engine::core::u32 binding,
                              engine::rhi::IVertexBuffer* buffer,
                              engine::core::usize offset = 0) override;
        void BindIndexBuffer(engine::rhi::IIndexBuffer* buffer,
                             engine::rhi::IndexType type,
                             engine::core::usize offset = 0) override;
        void SetVertexLayout(engine::core::u32 binding,
                             const engine::rhi::VertexLayoutDescription& layout) override;

        engine::rhi::IndexType GetIndexType() const noexcept override { return m_IndexType; }
        engine::core::u32 GetBoundVertexBufferCount() const noexcept override
        { return m_BoundVBCount; }

        [[nodiscard]] GLuint GetHandle() const noexcept { return m_Handle; }
        void Bind();

    private:
        GLuint                            m_Handle{0};
        std::string                       m_Name;
        OpenGLDebugLayer*                 m_DebugLayer{nullptr};
        engine::rhi::IndexType            m_IndexType{engine::rhi::IndexType::UInt32};
        engine::core::u32                 m_BoundVBCount{0};
    };

} // namespace engine::opengl
