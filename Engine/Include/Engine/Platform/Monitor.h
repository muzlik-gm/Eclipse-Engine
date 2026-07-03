// ============================================================================
// File: Engine/Include/Engine/Platform/Monitor.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <memory>
#include <string>
#include <vector>

namespace engine::platform {

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;

    // ========================================================================
    // VideoMode
    // ========================================================================
    struct VideoMode
    {
        u32 Width       = 0;
        u32 Height      = 0;
        i32 RefreshRate = 0;
        u8  RedBits     = 0;
        u8  GreenBits   = 0;
        u8  BlueBits    = 0;
    };

    // ========================================================================
    // MonitorInfo – snapshot of a monitor's capabilities and state
    // ========================================================================
    struct MonitorInfo
    {
        std::string         Name;
        bool                IsPrimary       = false;
        VideoMode           CurrentMode;
        i32                 PositionX       = 0;
        i32                 PositionY       = 0;
        u32                 PhysicalWidthMM = 0;
        u32                 PhysicalHeightMM = 0;
        f32                 ContentScaleX   = 1.0f;
        f32                 ContentScaleY   = 1.0f;
        i32                 WorkAreaX       = 0;
        i32                 WorkAreaY       = 0;
        u32                 WorkAreaWidth   = 0;
        u32                 WorkAreaHeight  = 0;
        std::vector<VideoMode> VideoModes;
    };

    // ========================================================================
    // IMonitor – pure-virtual monitor interface
    // ========================================================================
    class IMonitor : private engine::core::NonCopyable
    {
    public:
        virtual ~IMonitor() = default;

        [[nodiscard]] virtual const std::string& GetName() const = 0;
        [[nodiscard]] virtual bool              IsPrimary() const = 0;
        [[nodiscard]] virtual VideoMode         GetCurrentVideoMode() const = 0;
        [[nodiscard]] virtual std::vector<VideoMode> GetVideoModes() const = 0;
        virtual void GetPhysicalSize(u32& outWidthMM, u32& outHeightMM) const = 0;
        virtual void GetPosition(i32& outX, i32& outY) const = 0;
        virtual void GetContentScale(f32& outScaleX, f32& outScaleY) const = 0;
        virtual void GetWorkArea(i32& outX, i32& outY, u32& outWidth, u32& outHeight) const = 0;
        [[nodiscard]] virtual void* GetRawHandle() const = 0;

        // --------------------------------------------------------------------
        // Static queries – enumerate all connected monitors
        // --------------------------------------------------------------------
        [[nodiscard]] static std::vector<MonitorInfo> EnumerateMonitors();
        [[nodiscard]] static MonitorInfo              GetPrimaryMonitor();
    };

} // namespace engine::platform