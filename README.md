# Eclipse Engine

A professional, modular, cross-platform **C++20** game engine built with a strict layered architecture.

## Architecture

```
Platform  →  Core  →  Modules  →  Engine  →  Runtime  →  Editor
```

Each layer depends only on the layers to its left, ensuring clean separation of concerns and enabling independent testing at every level.

## Current Progress

| Phase | Status | Description |
|-------|--------|-------------|
| Phase 0 | Done | Core systems: logging, config, types, filesystem, threading, diagnostics |
| Phase 1 | Done | Runtime loop, subsystem manager, application framework |
| Phase 2 | Done | Platform layer: windowing (GLFW), events, monitors, clipboard, cursors |
| Phase 3 | Done | OpenGL 4.6 backend (implementation detail behind renderer interface) |
| Phase 4+ | Planned | Renderer abstractions, asset pipeline, ECS gameplay, editor, scripting... |

## Requirements

- **CMake** 3.22 or newer
- **C++20** compatible compiler:
  - GCC 13+ / 14+ (Linux)
  - MSVC 2022+ (Windows)
  - Clang 16+ (macOS)
- **Python 3** (for GLAD OpenGL loader generation)
- **Git** (for FetchContent dependency management)

### Platform-Specific Dependencies

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get install -y build-essential cmake python3 \
  libx11-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev libxfixes-dev libgl1-mesa-dev
```

**macOS:**
```bash
xcode-select --install
# CMake and Python 3 are available via Homebrew if not already present:
brew install cmake python3
```

**Windows:**
- Visual Studio 2022 with the C++ Desktop Development workload
- CMake (included with VS or install separately from cmake.org)
- Python 3 (from python.org or Microsoft Store)

## Building

```bash
# Clone the repository
git clone https://github.com/muzlik-gm/Eclipse-Engine.git
cd Eclipse-Engine

# Configure (Release build with tests enabled)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run tests
cd build && ctest --output-on-failure
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENGINE_BUILD_TESTS` | `ON` | Build the test suite |
| `ENGINE_BUILD_SANDBOX` | `OFF` | Build the sandbox application |
| `ENGINE_BUILD_EDITOR` | `OFF` | Build the editor |
| `ENGINE_BUILD_SHARED` | `OFF` | Build engine as a shared library (`.dll`/`.so`/`.dylib`) |
| `ENGINE_ENABLE_ASSERTIONS` | `ON` | Enable runtime assertions |
| `ENGINE_ENABLE_PROFILING` | `ON` | Enable profiling instrumentation |
| `ENGINE_ENABLE_LOGGING` | `ON` | Enable the logging system |
| `ENGINE_WARNINGS_AS_ERRORS` | `OFF` | Treat compiler warnings as errors |

Example — minimal build without tests:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENGINE_BUILD_TESTS=OFF
cmake --build build
```

### Clean Rebuild

If you ever need to build from a completely clean state:
```bash
rm -rf build _deps
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Dependencies

All dependencies are fetched automatically via CMake's FetchContent — no manual installation required.

| Library | Version | Purpose |
|---------|---------|---------|
| [spdlog](https://github.com/gabime/spdlog) | 1.15.1 | Fast structured logging |
| [fmt](https://github.com/fmtlib/fmt) | (via spdlog) | Modern C++ formatting |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | JSON configuration parsing |
| [yaml-cpp](https://github.com/jbeder/yaml-cpp) | 0.8.0 | YAML configuration parsing |
| [GLM](https://github.com/g-truc/glm) | 1.0.1 | OpenGL mathematics (header-only) |
| [EnTT](https://github.com/skypjack/entt) | 3.13.2 | Entity Component System (header-only) |
| [GLFW](https://github.com/glfw/glfw) | 3.3.9 | Cross-platform window and input |
| [GLAD](https://github.com/Dav1dde/glad) | 0.1.36 | OpenGL 4.6 Core Profile loader |
| [Google Test](https://github.com/google/googletest) | 1.15.2 | Unit testing framework (tests only) |

## Project Structure

```
Eclipse-Engine/
├── CMake/                  # CMake modules (platform detection, dependencies, etc.)
├── Engine/
│   ├── Include/Engine/     # Public headers (this is the engine API surface)
│   │   ├── Application/    # Application lifecycle
│   │   ├── Configuration/  # Config system (JSON + YAML)
│   │   ├── Core/           # Fundamental types, logging, assert, UUID, timing
│   │   ├── Diagnostics/    # Stack traces, crash handler, profiling, system info
│   │   ├── Engine.h        # Main engine include
│   │   ├── Events/         # Event bus and event types
│   │   ├── Filesystem/     # Path, file, and directory abstractions
│   │   ├── Math/           # Math types and utilities
│   │   ├── Memory/         # Memory management
│   │   ├── Platform/       # Window, monitor, clipboard, cursor interfaces
│   │   ├── Runtime/        # Engine loop, subsystem/module managers
│   │   ├── Threading/      # Thread, mutex, CPU info
│   │   └── Utilities/      # Hash, string, command-line parsing, environment
│   ├── Source/             # Implementation files
│   └── CMakeLists.txt      # Engine library target definition
├── Tests/                  # Google Test based unit tests
├── Sandbox/                # Standalone sandbox application
├── Editor/                 # Editor application (future)
├── ThirdParty/             # Compatibility shims (e.g., X11 header stubs for CI)
├── .github/workflows/      # CI pipelines
└── CMakeLists.txt          # Root build configuration
```

## Coding Conventions

- **Language**: C++20 throughout
- **Naming**: `PascalCase` for types and functions, `camelCase` for local variables, `m_` prefix for member variables, `kPascalCase` for constants
- **Platform detection**: Use `#include "Engine/Core/Platform.h"` and check `ENGINE_PLATFORM_WINDOWS`, `ENGINE_PLATFORM_LINUX`, `ENGINE_PLATFORM_MACOS` (defined as `0` or `1`)
- **Includes**: Always use the engine's public include path (`Engine/...`), never reach into `Source/` from outside the library
- **Error handling**: Use assertions via `ENGINE_ASSERT()`, logging via `ENGINE_LOG_*()` macros

## CI

GitHub Actions runs on every push and PR to `main`/`develop`:

| Job | Runner | Compiler |
|-----|--------|----------|
| Linux (GCC 13) | ubuntu-24.04 | gcc-13 |
| Linux (GCC 14) | ubuntu-24.04 | gcc-14 |
| Windows (MSVC) | windows-latest | MSVC 2022 |
| macOS (Clang) | macos-latest | Apple Clang |

Each job configures, builds, and runs the full test suite.

## License

See [LICENSE](LICENSE) for details.