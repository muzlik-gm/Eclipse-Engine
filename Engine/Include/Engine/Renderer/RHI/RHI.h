// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/RHI.h
// Umbrella header that includes the entire RHI public API in dependency
// order.  Including this single header gives downstream code access to
// every interface, description, and enum.
// ============================================================================
#pragma once

// -- Enums ---------------------------------------------------------------
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

// -- Descriptions --------------------------------------------------------
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"

// -- Interfaces (base first, then dependents in topological order) -------
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Interfaces/IBuffer.h"
#include "Engine/Renderer/RHI/Interfaces/ITexture.h"
#include "Engine/Renderer/RHI/Interfaces/IShader.h"
#include "Engine/Renderer/RHI/Interfaces/IPipeline.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"
#include "Engine/Renderer/RHI/Interfaces/ISync.h"
#include "Engine/Renderer/RHI/Interfaces/ICapabilities.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsDevice.h"

// -- Factories / backends -----------------------------------------------
#include "Engine/Renderer/RHI/Factories/IGraphicsFactory.h"
#include "Engine/Renderer/RHI/Factories/IGraphicsBackend.h"

// -- Validation ----------------------------------------------------------
#include "Engine/Renderer/RHI/Validation/IGraphicsValidation.h"

namespace engine::rhi {

    /// @brief Convenience alias: the RHI namespace exposes every public
    ///        type needed by the renderer and higher-level systems.
    ///
    /// No OpenGL / Vulkan / D3D / Metal symbols are visible here.  The
    /// active backend is selected at runtime through
    /// GraphicsBackendRegistry and accessed through the abstract
    /// interfaces above.

} // namespace engine::rhi
