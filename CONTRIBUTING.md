# Contributing to Eclipse Engine

## Debugging Methodology (for AI agents and humans)

When a test crashes or a CI job fails, follow this method strictly.
Do NOT skip steps. Do NOT guess the root cause before completing step 3.

### Step 1 — Identify the last output before the crash

Read the CI log or local test output. Find the **very last log line, print statement, or test assertion** that executed before the crash. This pinpoints exactly where execution stops.

```
[ RUN      ] ApplicationTest.InitializeAndShutdown
[I] Logging system initialized. App: TestApp   ← LAST OUTPUT. Crash is AFTER this.
```

If ctest reruns the failing test individually, that output is also shown — use it.

### Step 2 — Compare passing vs failing paths

Find a **similar test that passes** and list the exact code path for both, side by side. Identify every function call that differs between the two.

```
EngineTest.InitializeAndShutdown       → TEST_LOG_INIT() → Engine() → Engine::Initialize() → PASS
ApplicationTest.InitializeAndShutdown  → Application(4, argv) → ??? → CRASH
```

The divergence point is where the bug lives.

### Step 3 — Read every function in the failing path

From the last known good point to the crash, **read the actual source code** of every function called. Do not rely on assumptions about what a function "probably" does. Open the file and read it.

Pay special attention to:
- Array/pointer bounds (argc vs actual array size)
- C++ initializer list order (members initialize in **declaration order**, not initializer-list order)
- Object lifetimes (who owns what, when is it destroyed)
- Static initialization order
- Null pointer dereferences (does a macro/function guard against null?)
- Off-by-one errors in loops

### Step 4 — Verify locally with a clean build

Before pushing ANY fix, run:

```bash
rm -rf build _deps
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure --timeout 60
```

**All 178 tests must pass.** If even one fails, do not push. Fix it first.

### Step 5 — Commit and push only after full verification

```bash
git add -A
git diff --cached   # Review exactly what changed
git commit -m "fix: <one-line description of root cause>"
git push
```

## Anti-Patterns (learn from past mistakes)

### Do NOT diagnose based on assumptions

Past CI failures were incorrectly blamed on:
- Win32 console APIs in spdlog (actual cause: argc mismatch)
- `spdlog::shutdown()` vs `spdlog::drop()` (actual cause: same argc mismatch)
- Logging initialization order (actual cause: same argc mismatch)

All three "fixes" were wrong and two of them made things worse (broke Linux and macOS).

**Read the code first. Hypothesize second. Verify third.**

### Do NOT make platform-specific workarounds without proof

If a crash happens on only one platform, do NOT add `#if ENGINE_PLATFORM_WINDOWS` blocks
unless you have concrete evidence (stack trace, debugger output) that the platform API itself
is the problem. Cross-platform crashes are usually caused by undefined behavior (e.g.,
out-of-bounds access) that manifests differently per platform/compiler.

### Do NOT change shutdown/cleanup code to "fix" an init crash

If the crash happens during initialization, the bug is in the initialization path.
Changing shutdown code to compensate almost always causes regressions elsewhere.

### Do NOT push without clean-build verification

Stale build artifacts (cached object files, old `_deps/`) can mask bugs.
Always `rm -rf build _deps` before your final verification build.

## Architecture

```
Platform → Core → Modules → Engine → Runtime → Editor
```

- **Platform**: OS abstractions (Window, Monitor, Clipboard, etc.)
- **Core**: Foundational types, logging, filesystem, configuration
- **Runtime**: Engine lifecycle, subsystems, modules, main loop
- **Application**: Entry point, command-line parsing, window creation

## Build System

- CMake 3.22+
- All dependencies via FetchContent (spdlog, nlohmann_json, yaml-cpp, glm, entt, googletest, glfw, glad)
- No vendored dependencies in-tree (except ThirdParty/_glfw_x11_compat for X11 headers)

## Testing

```bash
# Full test suite
cmake --build build && cd build && ctest --output-on-failure --timeout 60

# Single test
cd build && ctest -R "ApplicationTest.InitializeAndShutdown" --output-on-failure

# With sanitizer (recommended for debugging memory issues)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
```