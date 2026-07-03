// ============================================================================
// File: Tests/Source/Platform/PlatformTests.cpp
// Tests for the Platform module — window properties, monitor info,
// platform info, dynamic library, clipboard, file dialog, and events.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Monitor.h"
#include "Engine/Platform/Cursor.h"
#include "Engine/Platform/DynamicLibrary.h"
#include "Engine/Platform/Clipboard.h"
#include "Engine/Platform/FileDialog.h"
#include "Engine/Platform/PlatformInfo.h"
#include "Engine/Events/Event.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace engine;
using namespace engine::platform;

// ============================================================================
// 1. WindowProperties defaults
// ============================================================================

TEST(WindowPropertiesTest, DefaultValues)
{
    WindowProperties props;

    EXPECT_EQ(props.Title, "Engine Window");
    EXPECT_EQ(props.Width, 1280u);
    EXPECT_EQ(props.Height, 720u);
    EXPECT_EQ(props.Mode, WindowMode::Windowed);
    EXPECT_TRUE(props.Resizable);
    EXPECT_FALSE(props.VSync);
    EXPECT_FLOAT_EQ(props.Opacity, 1.0f);
    EXPECT_TRUE(props.VisibleOnCreate);
}

// ============================================================================
// 2. WindowMode enum
// ============================================================================

TEST(WindowModeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(WindowMode::Windowed), 0);
    EXPECT_EQ(static_cast<int>(WindowMode::Fullscreen), 1);
    EXPECT_EQ(static_cast<int>(WindowMode::BorderlessFullscreen), 2);
}

// ============================================================================
// 3. CursorMode enum
// ============================================================================

TEST(CursorModeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(CursorMode::Normal), 0);
    EXPECT_EQ(static_cast<int>(CursorMode::Hidden), 1);
    EXPECT_EQ(static_cast<int>(CursorMode::Locked), 2);
}

// ============================================================================
// 4. CursorShape enum
// ============================================================================

TEST(CursorShapeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(CursorShape::Arrow), 0);
    EXPECT_EQ(static_cast<int>(CursorShape::IBeam), 1);
    EXPECT_EQ(static_cast<int>(CursorShape::Crosshair), 2);
    EXPECT_EQ(static_cast<int>(CursorShape::Hand), 3);
    EXPECT_EQ(static_cast<int>(CursorShape::HResize), 4);
    EXPECT_EQ(static_cast<int>(CursorShape::VResize), 5);
    EXPECT_EQ(static_cast<int>(CursorShape::ResizeAll), 6);
    EXPECT_EQ(static_cast<int>(CursorShape::NoCursor), 7);
}

// ============================================================================
// 5. VideoMode struct
// ============================================================================

TEST(VideoModeTest, DefaultConstruction)
{
    VideoMode mode;

    EXPECT_EQ(mode.Width, 0u);
    EXPECT_EQ(mode.Height, 0u);
    EXPECT_EQ(mode.RefreshRate, 0);
    EXPECT_EQ(mode.RedBits, static_cast<u8>(0));
    EXPECT_EQ(mode.GreenBits, static_cast<u8>(0));
    EXPECT_EQ(mode.BlueBits, static_cast<u8>(0));
}

TEST(VideoModeTest, FieldAssignment)
{
    VideoMode mode;
    mode.Width = 1920;
    mode.Height = 1080;
    mode.RefreshRate = 60;
    mode.RedBits = 8;
    mode.GreenBits = 8;
    mode.BlueBits = 8;

    EXPECT_EQ(mode.Width, 1920u);
    EXPECT_EQ(mode.Height, 1080u);
    EXPECT_EQ(mode.RefreshRate, 60);
    EXPECT_EQ(mode.RedBits, static_cast<u8>(8));
    EXPECT_EQ(mode.GreenBits, static_cast<u8>(8));
    EXPECT_EQ(mode.BlueBits, static_cast<u8>(8));
}

// ============================================================================
// 6. MonitorInfo struct
// ============================================================================

TEST(MonitorInfoTest, DefaultConstruction)
{
    MonitorInfo info;

    EXPECT_FALSE(info.IsPrimary);
    EXPECT_EQ(info.PositionX, 0);
    EXPECT_EQ(info.PositionY, 0);
    EXPECT_FLOAT_EQ(info.ContentScaleX, 1.0f);
    EXPECT_FLOAT_EQ(info.ContentScaleY, 1.0f);
}

TEST(MonitorInfoTest, DefaultFieldValues)
{
    MonitorInfo info;

    // Additional fields should have sensible defaults
    EXPECT_TRUE(info.Name.empty());
    EXPECT_EQ(info.PhysicalWidthMM, 0u);
    EXPECT_EQ(info.PhysicalHeightMM, 0u);
    EXPECT_EQ(info.WorkAreaX, 0);
    EXPECT_EQ(info.WorkAreaY, 0);
    EXPECT_EQ(info.WorkAreaWidth, 0u);
    EXPECT_EQ(info.WorkAreaHeight, 0u);
    EXPECT_TRUE(info.VideoModes.empty());
}

// ============================================================================
// 7. PlatformInfo static methods
// ============================================================================

TEST(PlatformInfoTest, GetPlatformName)
{
    std::string name = PlatformInfo::GetPlatformName();
    EXPECT_FALSE(name.empty());
#if ENGINE_PLATFORM_LINUX
    EXPECT_EQ(name, "Linux");
#elif ENGINE_PLATFORM_MACOS
    EXPECT_EQ(name, "macOS");
#elif ENGINE_PLATFORM_WINDOWS
    EXPECT_EQ(name, "Windows");
#endif
}

TEST(PlatformInfoTest, GetArchitecture)
{
    std::string arch = PlatformInfo::GetArchitecture();
    EXPECT_FALSE(arch.empty());
}

TEST(PlatformInfoTest, Is64Bit)
{
#if ENGINE_PLATFORM_64BIT
    EXPECT_TRUE(PlatformInfo::Is64Bit());
#else
    EXPECT_FALSE(PlatformInfo::Is64Bit());
#endif
}

TEST(PlatformInfoTest, Gather)
{
    PlatformInfo info = PlatformInfo::Gather();

    EXPECT_FALSE(info.OSName.empty());
    EXPECT_FALSE(info.Architecture.empty());
    EXPECT_FALSE(info.PlatformName.empty());
#if ENGINE_PLATFORM_64BIT
    EXPECT_TRUE(info.Is64BitPlatform);
#endif
}

TEST(PlatformInfoTest, GetEnginePlatformString)
{
    std::string str = PlatformInfo::GetEnginePlatformString();
    EXPECT_FALSE(str.empty());
}

// ============================================================================
// 8. DynamicLibrary
// ============================================================================

TEST(DynamicLibraryTest, CreateReturnsNonNull)
{
    auto lib = IDynamicLibrary::Create();
    ASSERT_NE(lib, nullptr);
}

TEST(DynamicLibraryTest, InitialState)
{
    auto lib = IDynamicLibrary::Create();
    ASSERT_NE(lib, nullptr);

    EXPECT_FALSE(lib->IsLoaded());
    EXPECT_TRUE(lib->GetFilePath().empty());
}

TEST(DynamicLibraryTest, GetSymbolReturnsNullptrWhenNotLoaded)
{
    auto lib = IDynamicLibrary::Create();
    ASSERT_NE(lib, nullptr);

    EXPECT_EQ(lib->GetSymbol("nonexistent"), nullptr);
}

TEST(DynamicLibraryTest, UnloadIsSafeWhenNotLoaded)
{
    auto lib = IDynamicLibrary::Create();
    ASSERT_NE(lib, nullptr);

    // Should not crash or throw when called on an unloaded library.
    EXPECT_NO_THROW(lib->Unload());

    // State should remain consistent.
    EXPECT_FALSE(lib->IsLoaded());
    EXPECT_TRUE(lib->GetFilePath().empty());
}

// ============================================================================
// 9. Clipboard interface
// ============================================================================

TEST(ClipboardTest, CreateReturnsNonNull)
{
    auto clipboard = IClipboard::Create();
    ASSERT_NE(clipboard, nullptr);
}

// ============================================================================
// 10. FileDialog interface
// ============================================================================

TEST(FileDialogTest, CreateReturnsNonNull)
{
    auto dialog = IFileDialog::Create();
    ASSERT_NE(dialog, nullptr);
}

TEST(FileDialogTest, OpenReturnsNullopt)
{
    auto dialog = IFileDialog::Create();
    ASSERT_NE(dialog, nullptr);

    auto result = dialog->Open(FileDialogType::OpenFile);
    EXPECT_FALSE(result.has_value());

    result = dialog->Open(FileDialogType::SaveFile);
    EXPECT_FALSE(result.has_value());
}

TEST(FileDialogTest, OpenMultipleReturnsEmpty)
{
    auto dialog = IFileDialog::Create();
    ASSERT_NE(dialog, nullptr);

    auto paths = dialog->OpenMultiple();
    EXPECT_TRUE(paths.empty());
}

// ============================================================================
// 11. Event types
// ============================================================================

using engine::events::Event;
using engine::events::EventCategory;
using engine::events::EventType;

TEST(EventTypeTest, AllWindowEventValues)
{
    EXPECT_EQ(static_cast<u32>(EventType::None), 0u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowClose), 1u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowResize), 2u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowFocus), 3u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowLostFocus), 4u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowMoved), 5u);
    EXPECT_EQ(static_cast<u32>(EventType::KeyPressed), 6u);
    EXPECT_EQ(static_cast<u32>(EventType::KeyReleased), 7u);
    EXPECT_EQ(static_cast<u32>(EventType::KeyTyped), 8u);
    EXPECT_EQ(static_cast<u32>(EventType::MouseMoved), 9u);
    EXPECT_EQ(static_cast<u32>(EventType::MouseScrolled), 10u);
    EXPECT_EQ(static_cast<u32>(EventType::MouseButtonPressed), 11u);
    EXPECT_EQ(static_cast<u32>(EventType::MouseButtonReleased), 12u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowMinimized), 13u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowMaximized), 14u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowRestored), 15u);
    EXPECT_EQ(static_cast<u32>(EventType::WindowRefresh), 16u);
    EXPECT_EQ(static_cast<u32>(EventType::FramebufferResized), 17u);
    EXPECT_EQ(static_cast<u32>(EventType::DPIChanged), 18u);
}

TEST(EventCategoryTest, FlagValues)
{
    EXPECT_EQ(static_cast<u32>(EventCategory::None), 0u);
    EXPECT_EQ(static_cast<u32>(EventCategory::Application), 1u << 0);
    EXPECT_EQ(static_cast<u32>(EventCategory::Input), 1u << 1);
    EXPECT_EQ(static_cast<u32>(EventCategory::Keyboard), 1u << 2);
    EXPECT_EQ(static_cast<u32>(EventCategory::Mouse), 1u << 3);
    EXPECT_EQ(static_cast<u32>(EventCategory::MouseButton), 1u << 4);
    EXPECT_EQ(static_cast<u32>(EventCategory::Window), 1u << 5);
}

// --- Individual window event GetName and GetCategoryFlags tests ---

TEST(WindowCreatedEventTest, NameAndCategory)
{
    engine::events::WindowCreatedEvent e(800, 600);
    EXPECT_EQ(e.GetName(), "WindowCreated");
    EXPECT_EQ(e.GetEventType(), EventType::None);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window | EventCategory::Application));
    EXPECT_EQ(e.GetWidth(), 800u);
    EXPECT_EQ(e.GetHeight(), 600u);
}

TEST(WindowCloseEventTest, NameAndCategory)
{
    engine::events::WindowCloseEvent e;
    EXPECT_EQ(e.GetName(), "WindowClose");
    EXPECT_EQ(e.GetEventType(), EventType::WindowClose);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
}

TEST(WindowResizeEventTest, NameAndCategory)
{
    engine::events::WindowResizeEvent e(1024, 768);
    EXPECT_EQ(e.GetName(), "WindowResize");
    EXPECT_EQ(e.GetEventType(), EventType::WindowResize);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
    EXPECT_EQ(e.GetWidth(), 1024u);
    EXPECT_EQ(e.GetHeight(), 768u);
}

TEST(WindowFocusEventTest, NameAndCategory)
{
    engine::events::WindowFocusEvent e;
    EXPECT_EQ(e.GetName(), "WindowFocus");
    EXPECT_EQ(e.GetEventType(), EventType::WindowFocus);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
}

TEST(WindowLostFocusEventTest, NameAndCategory)
{
    engine::events::WindowLostFocusEvent e;
    EXPECT_EQ(e.GetName(), "WindowLostFocus");
    EXPECT_EQ(e.GetEventType(), EventType::WindowLostFocus);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
}

TEST(WindowMovedEventTest, NameAndCategory)
{
    engine::events::WindowMovedEvent e(100, 200);
    EXPECT_EQ(e.GetName(), "WindowMoved");
    EXPECT_EQ(e.GetEventType(), EventType::WindowMoved);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
    EXPECT_EQ(e.GetX(), 100);
    EXPECT_EQ(e.GetY(), 200);
}

TEST(WindowMinimizedEventTest, NameAndCategory)
{
    engine::events::WindowMinimizedEvent e;
    EXPECT_EQ(e.GetName(), "WindowMinimized");
    EXPECT_EQ(e.GetEventType(), EventType::WindowMinimized);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
}

TEST(WindowMaximizedEventTest, NameAndCategory)
{
    engine::events::WindowMaximizedEvent e;
    EXPECT_EQ(e.GetName(), "WindowMaximized");
    EXPECT_EQ(e.GetEventType(), EventType::WindowMaximized);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
}

TEST(WindowRestoredEventTest, NameAndCategory)
{
    engine::events::WindowRestoredEvent e;
    EXPECT_EQ(e.GetName(), "WindowRestored");
    EXPECT_EQ(e.GetEventType(), EventType::WindowRestored);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
}

TEST(WindowRefreshEventTest, NameAndCategory)
{
    engine::events::WindowRefreshEvent e;
    EXPECT_EQ(e.GetName(), "WindowRefresh");
    EXPECT_EQ(e.GetEventType(), EventType::WindowRefresh);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
}

TEST(FramebufferResizeEventTest, NameAndCategory)
{
    engine::events::FramebufferResizeEvent e(1920, 1080);
    EXPECT_EQ(e.GetName(), "FramebufferResize");
    EXPECT_EQ(e.GetEventType(), EventType::FramebufferResized);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
    EXPECT_EQ(e.GetWidth(), 1920u);
    EXPECT_EQ(e.GetHeight(), 1080u);
}

TEST(DPIChangedEventTest, NameAndCategory)
{
    engine::events::DPIChangedEvent e(1.5f, 1.5f);
    EXPECT_EQ(e.GetName(), "DPIChanged");
    EXPECT_EQ(e.GetEventType(), EventType::DPIChanged);
    EXPECT_EQ(static_cast<u32>(e.GetCategoryFlags()),
              static_cast<u32>(EventCategory::Window));
    EXPECT_FLOAT_EQ(e.GetScaleX(), 1.5f);
    EXPECT_FLOAT_EQ(e.GetScaleY(), 1.5f);
}

TEST(EventTest, IsInCategory)
{
    engine::events::WindowCloseEvent closeEvent;
    EXPECT_TRUE(closeEvent.IsInCategory(EventCategory::Window));
    EXPECT_FALSE(closeEvent.IsInCategory(EventCategory::Input));
    EXPECT_FALSE(closeEvent.IsInCategory(EventCategory::Keyboard));
    EXPECT_FALSE(closeEvent.IsInCategory(EventCategory::Mouse));
    EXPECT_FALSE(closeEvent.IsInCategory(EventCategory::MouseButton));

    engine::events::WindowCreatedEvent createdEvent(800, 600);
    EXPECT_TRUE(createdEvent.IsInCategory(EventCategory::Window));
    EXPECT_TRUE(createdEvent.IsInCategory(EventCategory::Application));
}

TEST(EventTest, HandledFlag)
{
    engine::events::WindowCloseEvent e;
    EXPECT_FALSE(e.IsHandled());
    e.SetHandled(true);
    EXPECT_TRUE(e.IsHandled());
    e.SetHandled(false);
    EXPECT_FALSE(e.IsHandled());
}

// ============================================================================
// 12. WindowProperties modification
// ============================================================================

TEST(WindowPropertiesTest, Modification)
{
    WindowProperties props;

    props.Title = "Custom Title";
    props.Width = 1920;
    props.Height = 1080;
    props.Mode = WindowMode::Fullscreen;
    props.Resizable = false;
    props.VSync = true;
    props.MinWidth = 640;
    props.MinHeight = 480;
    props.MaxWidth = 3840;
    props.MaxHeight = 2160;
    props.Opacity = 0.75f;
    props.VisibleOnCreate = false;

    EXPECT_EQ(props.Title, "Custom Title");
    EXPECT_EQ(props.Width, 1920u);
    EXPECT_EQ(props.Height, 1080u);
    EXPECT_EQ(props.Mode, WindowMode::Fullscreen);
    EXPECT_FALSE(props.Resizable);
    EXPECT_TRUE(props.VSync);
    EXPECT_EQ(props.MinWidth, 640u);
    EXPECT_EQ(props.MinHeight, 480u);
    EXPECT_EQ(props.MaxWidth, 3840u);
    EXPECT_EQ(props.MaxHeight, 2160u);
    EXPECT_FLOAT_EQ(props.Opacity, 0.75f);
    EXPECT_FALSE(props.VisibleOnCreate);
}

TEST(WindowPropertiesTest, BorderlessFullscreenMode)
{
    WindowProperties props;
    props.Mode = WindowMode::BorderlessFullscreen;
    EXPECT_EQ(props.Mode, WindowMode::BorderlessFullscreen);
}