// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLPipeline.cpp
// ============================================================================
#include "OpenGLPipeline.h"
#include "OpenGLShader.h"
#include "OpenGLTypes.h"
#include "OpenGLStateCache.h"
#include "OpenGLDebugLayer.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    using namespace engine::rhi;

    // ========================================================================
    // OpenGLPipelineLayout
    // ========================================================================

    OpenGLPipelineLayout::OpenGLPipelineLayout(const PipelineLayoutDescription& desc,
                                                 OpenGLDebugLayer* /*debugLayer*/)
        : m_Description(desc)
        , m_Name("PipelineLayout")
    {
    }

    // ========================================================================
    // OpenGLGraphicsPipeline
    // ========================================================================

    OpenGLGraphicsPipeline::OpenGLGraphicsPipeline(const GraphicsPipelineDescription& desc,
                                                     OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_Name(desc.Name)
        , m_DebugLayer(debugLayer)
    {
        // Create the pipeline layout.
        m_Layout = std::make_unique<OpenGLPipelineLayout>(desc.Layout, debugLayer);

        // Create and compile the shader program.
        m_Shader = std::make_unique<OpenGLShader>(desc.Shader, debugLayer);

        Validate();
    }

    engine::rhi::IShader* OpenGLGraphicsPipeline::GetShader() const noexcept
    {
        return static_cast<engine::rhi::IShader*>(m_Shader.get());
    }

    void OpenGLGraphicsPipeline::Validate()
    {
        if (!m_Shader || !m_Shader->IsLinked())
        {
            m_ValidationLog = "Shader program failed to link";
            m_Valid = false;
            ENGINE_LOG_ERROR("OpenGLGraphicsPipeline — shader not linked for '{}'", m_Name);
            return;
        }

        // Validate vertex layout against shader attributes (basic check).
        for (const auto& binding : m_Description.VertexLayout.Bindings)
        {
            if (binding.Stride == 0)
            {
                m_ValidationLog = "Vertex binding " + std::to_string(binding.Binding)
                                  + " has zero stride";
                m_Valid = false;
                return;
            }
        }

        m_Valid = true;
        ENGINE_LOG_DEBUG("OpenGLGraphicsPipeline — validated '{}' (shader handle={})",
                         m_Name, m_Shader->GetHandle());
    }

    void OpenGLGraphicsPipeline::Apply(OpenGLStateCache& stateCache)
    {
        if (!m_Valid || !m_Shader)
            return;

        // Bind the shader program.
        if (stateCache.SetProgram(m_Shader->GetHandle()))
        {
            glUseProgram(m_Shader->GetHandle());
        }

        // -- Rasterizer state ---------------------------------------------
        const auto& rs = m_Description.RasterizerState;

        if (stateCache.SetDepthClampEnabled(rs.DepthClampEnable))
        {
            if (rs.DepthClampEnable) glEnable(GL_DEPTH_CLAMP);
            else glDisable(GL_DEPTH_CLAMP);
        }

        if (rs.RasterizerDiscardEnable)
        {
            glEnable(GL_RASTERIZER_DISCARD);
        }
        else
        {
            glDisable(GL_RASTERIZER_DISCARD);
        }

        if (rs.CullModeValue != CullMode::None)
        {
            if (stateCache.SetCullFaceEnabled(true)) glEnable(GL_CULL_FACE);
            if (stateCache.SetCullMode(ToGLCullMode(rs.CullModeValue)))
                glCullFace(ToGLCullMode(rs.CullModeValue));
        }
        else
        {
            if (stateCache.SetCullFaceEnabled(false)) glDisable(GL_CULL_FACE);
        }

        if (stateCache.SetFrontFace(ToGLFrontFace(rs.FrontFaceValue)))
            glFrontFace(ToGLFrontFace(rs.FrontFaceValue));

        if (stateCache.SetPolygonMode(ToGLPolygonMode(rs.PolygonModeValue)))
            glPolygonMode(GL_FRONT_AND_BACK, ToGLPolygonMode(rs.PolygonModeValue));

        if (rs.DepthBiasEnable)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glEnable(GL_POLYGON_OFFSET_POINT);
            glPolygonOffset(rs.DepthBiasSlopeFactor, rs.DepthBiasConstantFactor);
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_POLYGON_OFFSET_LINE);
            glDisable(GL_POLYGON_OFFSET_POINT);
        }

        glLineWidth(rs.LineWidth);

        // -- Blend state --------------------------------------------------
        const auto& bs = m_Description.BlendState;
        const bool blendEnabled = !bs.Attachments.empty() && bs.Attachments[0].BlendEnable;

        if (stateCache.SetBlendEnabled(blendEnabled))
        {
            if (blendEnabled) glEnable(GL_BLEND);
            else glDisable(GL_BLEND);
        }

        if (blendEnabled && !bs.Attachments.empty())
        {
            const auto& att = bs.Attachments[0];
            if (stateCache.SetBlendFunc(
                    ToGLBlendFactor(att.SrcColorBlendFactor),
                    ToGLBlendFactor(att.DstColorBlendFactor),
                    ToGLBlendFactor(att.SrcAlphaBlendFactor),
                    ToGLBlendFactor(att.DstAlphaBlendFactor)))
            {
                glBlendFuncSeparate(
                    ToGLBlendFactor(att.SrcColorBlendFactor),
                    ToGLBlendFactor(att.DstColorBlendFactor),
                    ToGLBlendFactor(att.SrcAlphaBlendFactor),
                    ToGLBlendFactor(att.DstAlphaBlendFactor));
            }

            if (stateCache.SetBlendEquation(
                    ToGLBlendOperation(att.ColorBlendOp),
                    ToGLBlendOperation(att.AlphaBlendOp)))
            {
                glBlendEquationSeparate(
                    ToGLBlendOperation(att.ColorBlendOp),
                    ToGLBlendOperation(att.AlphaBlendOp));
            }

            if (stateCache.SetBlendColor(
                    bs.BlendConstants[0], bs.BlendConstants[1],
                    bs.BlendConstants[2], bs.BlendConstants[3]))
            {
                glBlendColor(bs.BlendConstants[0], bs.BlendConstants[1],
                             bs.BlendConstants[2], bs.BlendConstants[3]);
            }
        }

        // -- Depth / stencil state ---------------------------------------
        const auto& ds = m_Description.DepthStencilState;

        if (stateCache.SetDepthTestEnabled(ds.DepthTestEnable))
        {
            if (ds.DepthTestEnable) glEnable(GL_DEPTH_TEST);
            else glDisable(GL_DEPTH_TEST);
        }

        if (stateCache.SetDepthFunc(ToGLCompareOperation(ds.DepthCompareOp)))
        {
            glDepthFunc(ToGLCompareOperation(ds.DepthCompareOp));
        }

        if (stateCache.SetDepthWriteEnabled(ds.DepthWriteEnable))
        {
            glDepthMask(ds.DepthWriteEnable ? GL_TRUE : GL_FALSE);
        }

        if (stateCache.SetStencilTestEnabled(ds.StencilTestEnable))
        {
            if (ds.StencilTestEnable) glEnable(GL_STENCIL_TEST);
            else glDisable(GL_STENCIL_TEST);
        }

        if (ds.StencilTestEnable)
        {
            const auto& front = ds.FrontStencil;
            const auto& back  = ds.BackStencil;

            if (stateCache.SetStencilFunc(GL_FRONT,
                    ToGLCompareOperation(front.CompareOp),
                    static_cast<GLint>(front.Reference),
                    front.CompareMask))
            {
                glStencilFuncSeparate(GL_FRONT,
                    ToGLCompareOperation(front.CompareOp),
                    static_cast<GLint>(front.Reference),
                    front.CompareMask);
            }

            if (stateCache.SetStencilOp(GL_FRONT,
                    ToGLStencilOperation(front.FailOp),
                    ToGLStencilOperation(front.DepthFailOp),
                    ToGLStencilOperation(front.PassOp)))
            {
                glStencilOpSeparate(GL_FRONT,
                    ToGLStencilOperation(front.FailOp),
                    ToGLStencilOperation(front.DepthFailOp),
                    ToGLStencilOperation(front.PassOp));
            }

            if (stateCache.SetStencilFunc(GL_BACK,
                    ToGLCompareOperation(back.CompareOp),
                    static_cast<GLint>(back.Reference),
                    back.CompareMask))
            {
                glStencilFuncSeparate(GL_BACK,
                    ToGLCompareOperation(back.CompareOp),
                    static_cast<GLint>(back.Reference),
                    back.CompareMask);
            }

            if (stateCache.SetStencilOp(GL_BACK,
                    ToGLStencilOperation(back.FailOp),
                    ToGLStencilOperation(back.DepthFailOp),
                    ToGLStencilOperation(back.PassOp)))
            {
                glStencilOpSeparate(GL_BACK,
                    ToGLStencilOperation(back.FailOp),
                    ToGLStencilOperation(back.DepthFailOp),
                    ToGLStencilOperation(back.PassOp));
            }

            if (stateCache.SetStencilMask(GL_FRONT, front.WriteMask))
                glStencilMaskSeparate(GL_FRONT, front.WriteMask);
            if (stateCache.SetStencilMask(GL_BACK, back.WriteMask))
                glStencilMaskSeparate(GL_BACK, back.WriteMask);
        }
    }

} // namespace engine::opengl
