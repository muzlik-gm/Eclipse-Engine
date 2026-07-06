// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLCommandQueue.cpp
// ============================================================================
#include "OpenGLCommandQueue.h"
#include "OpenGLContext.h"
#include "OpenGLDebugLayer.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    OpenGLCommandQueue::OpenGLCommandQueue(engine::rhi::QueueType type,
                                            OpenGLContext* context,
                                            OpenGLDebugLayer* debugLayer)
        : m_Type(type), m_Context(context), m_DebugLayer(debugLayer)
    {
    }

    void OpenGLCommandQueue::Submit(std::span<engine::rhi::ICommandBuffer* const> /*commandBuffers*/)
    {
        // OpenGL command buffers execute immediately — there is no deferred
        // submission.  This method exists for interface completeness.
    }

    void OpenGLCommandQueue::WaitIdle()
    {
        glFinish();
    }

    void OpenGLCommandQueue::Present(engine::rhi::ISwapChain* /*swapchain*/)
    {
        if (m_Context)
            m_Context->SwapBuffers();
    }

} // namespace engine::opengl
