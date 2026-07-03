// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWMonitor.cpp
// GLFW-backed implementation of monitor enumeration and queries.
// ============================================================================

#include "Engine/Platform/Monitor.h"
#include "Engine/Core/Log.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace engine::platform
{

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;
    using engine::core::u8;
    using engine::core::usize;

    // ========================================================================
    // Helper — convert GLFWvidmode to VideoMode
    // ========================================================================

    static VideoMode GLFWVideoModeToEngine(const GLFWvidmode& mode)
    {
        VideoMode vm;
        vm.Width       = static_cast<u32>(mode.width);
        vm.Height      = static_cast<u32>(mode.height);
        vm.RefreshRate = mode.refreshRate;
        vm.RedBits     = static_cast<u8>(mode.redBits);
        vm.GreenBits   = static_cast<u8>(mode.greenBits);
        vm.BlueBits    = static_cast<u8>(mode.blueBits);
        return vm;
    }

    // ========================================================================
    // Helper — gather info for one monitor
    // ========================================================================

    static MonitorInfo GatherMonitorInfo(GLFWmonitor* monitor, bool isPrimary)
    {
        MonitorInfo info;
        info.IsPrimary = isPrimary;

        const char* name = glfwGetMonitorName(monitor);
        info.Name = name ? std::string(name) : "Unknown Monitor";

        int xpos = 0, ypos = 0;
        glfwGetMonitorPos(monitor, &xpos, &ypos);
        info.PositionX = xpos;
        info.PositionY = ypos;

        int widthMM = 0, heightMM = 0;
        glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
        info.PhysicalWidthMM  = static_cast<u32>(widthMM);
        info.PhysicalHeightMM = static_cast<u32>(heightMM);

        float sx = 1.0f, sy = 1.0f;
        glfwGetMonitorContentScale(monitor, &sx, &sy);
        info.ContentScaleX = static_cast<f32>(sx);
        info.ContentScaleY = static_cast<f32>(sy);

        int wx = 0, wy = 0, ww = 0, wh = 0;
        glfwGetMonitorWorkarea(monitor, &wx, &wy, &ww, &wh);
        info.WorkAreaX      = wx;
        info.WorkAreaY      = wy;
        info.WorkAreaWidth  = static_cast<u32>(ww);
        info.WorkAreaHeight = static_cast<u32>(wh);

        const GLFWvidmode* currentMode = glfwGetVideoMode(monitor);
        if (currentMode)
        {
            info.CurrentMode = GLFWVideoModeToEngine(*currentMode);
        }

        int modeCount = 0;
        const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
        info.VideoModes.reserve(static_cast<usize>(modeCount));
        for (int i = 0; i < modeCount; ++i)
        {
            info.VideoModes.push_back(GLFWVideoModeToEngine(modes[i]));
        }

        return info;
    }

    // ========================================================================
    // Static queries
    // ========================================================================

    std::vector<MonitorInfo> IMonitor::EnumerateMonitors()
    {
        std::vector<MonitorInfo> result;

        int count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        if (!monitors || count <= 0)
        {
            ENGINE_LOG_WARN("GLFWMonitor — no monitors found");
            return result;
        }

        GLFWmonitor* primary = glfwGetPrimaryMonitor();

        result.reserve(static_cast<usize>(count));
        for (int i = 0; i < count; ++i)
        {
            bool isPrimary = (monitors[i] == primary);
            result.push_back(GatherMonitorInfo(monitors[i], isPrimary));
        }

        return result;
    }

    MonitorInfo IMonitor::GetPrimaryMonitor()
    {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (!primary)
        {
            ENGINE_LOG_WARN("GLFWMonitor — no primary monitor found");
            return MonitorInfo{};
        }

        return GatherMonitorInfo(primary, true);
    }

} // namespace engine::platform
