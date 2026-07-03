// ============================================================================
// File: Engine/Source/Platform/GLFW/GLFWDynamicLibrary.cpp
// Platform-specific implementation of IDynamicLibrary using dlopen/dlsym.
// ============================================================================

#include "Engine/Platform/DynamicLibrary.h"
#include "Engine/Core/Log.h"

#ifdef ENGINE_PLATFORM_LINUX
    #include <dlfcn.h>
#elif defined(ENGINE_PLATFORM_MACOS)
    #include <dlfcn.h>
#elif defined(ENGINE_PLATFORM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

namespace engine::platform
{

    // ========================================================================
    // PlatformDynamicLibrary
    // ========================================================================

    class PlatformDynamicLibrary final : public IDynamicLibrary
    {
    public:
        PlatformDynamicLibrary()  = default;
        ~PlatformDynamicLibrary() override { Unload(); }

        PlatformDynamicLibrary(const PlatformDynamicLibrary&)            = delete;
        PlatformDynamicLibrary& operator=(const PlatformDynamicLibrary&) = delete;
        PlatformDynamicLibrary(PlatformDynamicLibrary&&)                 = delete;
        PlatformDynamicLibrary& operator=(PlatformDynamicLibrary&&)      = delete;

        bool Load(const std::string& filePath) override
        {
            if (m_Handle)
            {
                ENGINE_LOG_WARN("DynamicLibrary — already loaded '{}', unloading first", m_FilePath);
                Unload();
            }

#ifdef ENGINE_PLATFORM_LINUX
            m_Handle = dlopen(filePath.c_str(), RTLD_NOW);
#elif defined(ENGINE_PLATFORM_MACOS)
            m_Handle = dlopen(filePath.c_str(), RTLD_NOW);
#elif defined(ENGINE_PLATFORM_WINDOWS)
            m_Handle = static_cast<void*>(LoadLibraryA(filePath.c_str()));
#endif

            if (!m_Handle)
            {
#ifdef ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
                ENGINE_LOG_ERROR("DynamicLibrary — failed to load '{}': {}", filePath, dlerror());
#elif defined(ENGINE_PLATFORM_WINDOWS)
                ENGINE_LOG_ERROR("DynamicLibrary — failed to load '{}'", filePath);
#endif
                return false;
            }

            m_FilePath = filePath;
            ENGINE_LOG_DEBUG("DynamicLibrary — loaded '{}'", filePath);
            return true;
        }

        void Unload() override
        {
            if (!m_Handle) return;

#ifdef ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
            dlclose(m_Handle);
#elif defined(ENGINE_PLATFORM_WINDOWS)
            FreeLibrary(static_cast<HMODULE>(m_Handle));
#endif

            ENGINE_LOG_DEBUG("DynamicLibrary — unloaded '{}'", m_FilePath);
            m_Handle   = nullptr;
            m_FilePath.clear();
        }

        [[nodiscard]] void* GetSymbol(const std::string& name) const override
        {
            if (!m_Handle) return nullptr;

#ifdef ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
            return dlsym(m_Handle, name.c_str());
#elif defined(ENGINE_PLATFORM_WINDOWS)
            return static_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_Handle), name.c_str()));
#else
            return nullptr;
#endif
        }

        [[nodiscard]] bool IsLoaded() const override
        {
            return m_Handle != nullptr;
        }

        [[nodiscard]] const std::string& GetFilePath() const override
        {
            return m_FilePath;
        }

    private:
        void*       m_Handle   = nullptr;
        std::string m_FilePath;
    };

    // ========================================================================
    // IDynamicLibrary factory
    // ========================================================================

    std::unique_ptr<IDynamicLibrary> IDynamicLibrary::Create()
    {
        return std::make_unique<PlatformDynamicLibrary>();
    }

} // namespace engine::platform
