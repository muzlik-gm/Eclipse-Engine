// ============================================================================
// File: Engine/Source/Diagnostics/StackTrace.cpp
// Full implementation of StackTrace — platform-specific back-trace capture
// and C++ symbol demangling.
//
// Platform notes:
//   Linux  : <execinfo.h> backtrace() / backtrace_symbols() + __cxa_demangle
//   macOS  : same as Linux (execinfo is available via libunwind / libc)
//   Windows: CaptureStackBackTrace() + DbgHelp (SymFromAddr, UnDecorateSymbolName)
//
// Linking note (Linux/macOS): link with -rdynamic (or -Wl,--export-dynamic)
//   so that backtrace_symbols() can resolve function names.
// ============================================================================

#include "Engine/Diagnostics/StackTrace.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Platform.h"

#include <sstream>
#include <iomanip>

// ============================================================================
// Platform-specific includes
// ============================================================================

#if ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
    #include <execinfo.h>
    #include <cxxabi.h>
    #include <cstdlib>
    #include <cstring>
#elif ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <dbghelp.h>
    #pragma comment(lib, "dbghelp.lib")
#endif

namespace engine::diagnostics
{

// ===========================================================================
// Demangle
// ===========================================================================

std::string StackTrace::Demangle(const std::string& mangled)
{
#if ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
    // __cxa_demangle is available with GCC and Clang on POSIX.
    int status = 0;
    char* demangled = abi::__cxa_demangle(
        mangled.c_str(),
        nullptr,   // output_buffer
        nullptr,   // length
        &status);

    if (demangled != nullptr && status == 0)
    {
        std::string result(demangled);
        std::free(demangled);
        return result;
    }

    // Demangling failed — return the original string.
    if (demangled != nullptr)
    {
        std::free(demangled);
    }
    return mangled;
#elif ENGINE_PLATFORM_WINDOWS
    // MSVC uses a different mangling scheme. UnDecorateSymbolName handles it.
    // Allocate a generously-sized buffer; truncate if needed.
    char undecorated[1024] = {};
    DWORD resultLen = UnDecorateSymbolName(
        mangled.c_str(),
        undecorated,
        static_cast<DWORD>(sizeof(undecorated)),
        UNDNAME_COMPLETE);

    if (resultLen > 0)
    {
        return std::string(undecorated, resultLen);
    }
    return mangled;
#else
    return mangled;
#endif
}

// ===========================================================================
// Capture — POSIX (Linux & macOS)
// ===========================================================================

#if ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS

std::string StackTrace::Capture(usize maxFrames)
{
    // Allocate the buffer for raw address pointers.
    std::vector<void*> buffer(maxFrames);
    int frameCount = ::backtrace(buffer.data(), static_cast<int>(maxFrames));

    if (frameCount <= 0)
    {
        return "<no stack frames captured>";
    }

    // Convert addresses to symbol strings.
    char** symbols = ::backtrace_symbols(buffer.data(), frameCount);
    if (symbols == nullptr)
    {
        return "<failed to resolve stack symbols>";
    }

    std::ostringstream oss;
    oss << "Stack trace (" << frameCount << " frames):\n";

    for (int i = 0; i < frameCount; ++i)
    {
        oss << "  [" << std::setw(3) << i << "] ";

        // backtrace_symbols returns strings of the form:
        //   <binary>(<mangled+offset>) [<address>]
        // We try to extract and demangle the function name.
        std::string raw(symbols[i]);

        // Look for the opening '(' which precedes the mangled name.
        std::size_t openParen  = raw.find('(');
        std::size_t closeParen = raw.find(')');

        if (openParen != std::string::npos && closeParen != std::string::npos
            && closeParen > openParen)
        {
            // Extract the part inside parentheses, e.g. "_ZN3foo4barEi+0x1a"
            std::string mangledWithOffset = raw.substr(openParen + 1, closeParen - openParen - 1);

            // Strip the "+0x..." offset suffix if present.
            std::size_t plusPos = mangledWithOffset.find('+');
            std::string mangledName = (plusPos != std::string::npos)
                ? mangledWithOffset.substr(0, plusPos)
                : mangledWithOffset;

            // Demangle if it looks like a C++ symbol (starts with '_Z').
            std::string demangled = (!mangledName.empty() && mangledName[0] == '_')
                ? Demangle(mangledName)
                : mangledName;

            oss << demangled;
            if (plusPos != std::string::npos)
            {
                oss << " " << mangledWithOffset.substr(plusPos);
            }
        }
        else
        {
            oss << raw;
        }

        oss << "\n";
    }

    std::free(symbols);
    return oss.str();
}

#endif // POSIX

// ===========================================================================
// Capture — Windows
// ===========================================================================

#if ENGINE_PLATFORM_WINDOWS

namespace
{

/// RAII wrapper for the DbgHelp symbol handle.
struct DbgHelpSession
{
    bool initialized = false;

    DbgHelpSession()
    {
        // SymInitialize only needs to be called once per process.
        initialized = (SymInitialize(GetCurrentProcess(), nullptr, TRUE) == TRUE);
    }

    ~DbgHelpSession()
    {
        if (initialized)
        {
            SymCleanup(GetCurrentProcess());
        }
    }
};

// Process-wide DbgHelp session (lazy, thread-safe in C++20).
// We use a function-local static so initialization happens on first use.
DbgHelpSession& GetDbgHelpSession()
{
    static DbgHelpSession session;
    return session;
}

} // anonymous namespace

std::string StackTrace::Capture(usize maxFrames)
{
    // Clamp to the API limit.
    if (maxFrames > 62)
    {
        maxFrames = 62;
    }

    void* buffer[64] = {};
    WORD frameCount = CaptureStackBackTrace(
        static_cast<DWORD>(1),  // skip this function
        static_cast<DWORD>(maxFrames),
        buffer,
        nullptr);

    if (frameCount == 0)
    {
        return "<no stack frames captured>";
    }

    // Ensure DbgHelp is initialised.
    DbgHelpSession& dbgHelp = GetDbgHelpSession();
    if (!dbgHelp.initialized)
    {
        // SymInitialize failed — return raw addresses only.
        std::ostringstream oss;
        oss << "Stack trace (" << frameCount << " frames, symbols unavailable):\n";
        for (WORD i = 0; i < frameCount; ++i)
        {
            oss << "  [" << std::setw(3) << i << "] 0x"
                << std::hex << std::setfill('0')
                << std::setw(static_cast<int>(sizeof(void*)) * 2)
                << reinterpret_cast<std::uintptr_t>(buffer[i])
                << std::dec << std::setfill(' ') << "\n";
        }
        return oss.str();
    }

    HANDLE process = GetCurrentProcess();
    std::ostringstream oss;
    oss << "Stack trace (" << frameCount << " frames):\n";

    // Allocate symbol info buffer with room for the name.
    // SymFromAddr requires sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR).
    constexpr usize kMaxNameLen = 256;
    auto symbolInfoBuf = std::make_unique<unsigned char[]>(sizeof(SYMBOL_INFO) + kMaxNameLen);
    auto* symbol       = reinterpret_cast<SYMBOL_INFO*>(symbolInfoBuf.get());
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen   = static_cast<ULONG>(kMaxNameLen);

    DWORD64 displacement64 = 0;
    IMAGEHLP_LINE64 line = {};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    for (WORD i = 0; i < frameCount; ++i)
    {
        oss << "  [" << std::setw(3) << i << "] 0x"
            << std::hex << std::setfill('0')
            << std::setw(static_cast<int>(sizeof(void*)) * 2)
            << reinterpret_cast<std::uintptr_t>(buffer[i])
            << std::dec << std::setfill(' ');

        BOOL hasSymbol = SymFromAddr(process, reinterpret_cast<DWORD64>(buffer[i]), &displacement64, symbol);
        if (hasSymbol)
        {
            oss << " " << symbol->Name;
            if (displacement64 != 0)
            {
                oss << " + 0x" << std::hex << displacement64 << std::dec;
            }
        }

        DWORD lineDisplacement = 0;
        BOOL hasLine = SymGetLineFromAddr64(process, reinterpret_cast<DWORD64>(buffer[i]), &lineDisplacement, &line);
        if (hasLine)
        {
            oss << "  (" << line.FileName << ":" << line.LineNumber << ")";
        }

        oss << "\n";
    }

    return oss.str();
}

#endif // _WIN32

// ===========================================================================
// Fallback Capture for unknown platforms
// ===========================================================================

#if !ENGINE_PLATFORM_LINUX && !ENGINE_PLATFORM_MACOS && !ENGINE_PLATFORM_WINDOWS

std::string StackTrace::Capture(usize /*maxFrames*/)
{
    return "<stack trace not supported on this platform>";
}

#endif

// ===========================================================================
// Print
// ===========================================================================

void StackTrace::Print()
{
    std::string trace = Capture();
    // Log each line individually so spdlog handles newlines properly
    // and the output is timestamped per-frame.
    std::istringstream stream(trace);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty())
        {
            ENGINE_LOG_ERROR("{}", line);
        }
    }
}

} // namespace engine::diagnostics