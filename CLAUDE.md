# GamePort - TUI Game Launcher

C99 terminal application that discovers, installs, and launches open-source games.

## Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug    # configure
cmake --build build                         # build
./build/gameport                            # run
```

Release build: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`

Smoke test: `timeout 1 ./build/gameport` — exit code 124 (killed by timeout) is normal, not a crash.

Cross-platform: CMake handles Linux (GCC/Clang), macOS (AppleClang), and Windows (MSVC).
Version is set in `CMakeLists.txt` via `project(VERSION X.Y.Z)` and passed as `GAMEPORT_VERSION` define.

## Architecture

```
src/
  main.c           — event loop, install/uninstall/launch logic
  ui.c / ui.h      — rendering, App state, filtering, categories
  catalog.c/.h     — Game/Source structs, platform detection, shared constants
  util.c / util.h  — toolchain detection, path helpers, install tracking
  install.c/.h     — ChildProc, LineBuf, child process management
  term.c / term.h  — terminal abstraction (Screen, KeyEvent, colors)
  platform.h       — cross-platform API (PlatProc, filesystem, terminal)
  platform_posix.c — POSIX implementation
  platform_win32.c — Windows implementation
  games/
    registry.c     — master game list (function pointer table)
    *.c            — one file per game (static data, no logic)
```

## Key Conventions

### Adding a Game

1. Create `src/games/<name>.c` with static data: keys, sources, deps, game_data
2. Add `const Game *game_<name>(void);` declaration and entry in `src/games/registry.c`
3. CMake `file(GLOB)` picks up new .c files automatically — no CMakeLists.txt changes needed

### Game File Template

Every game file follows the same pattern:
```c
#include "../catalog.h"
static const char *keys[] = { "Key|Action", ..., NULL };
static const Source sources[] = {{ .method = ..., .label = "...", ... }};
static const Game game_data = { .name = "...", .icon = "X", ... };
const Game *game_<name>(void) { return &game_data; }
```

### Acquisition Methods

- `ACQUIRE_CARGO` — `cargo install <bin>` (cross-platform, needs Rust)
- `ACQUIRE_GIT` — `git clone` + build commands (needs git + build tools)
- `ACQUIRE_DOWNLOAD` — `curl` download of pre-built binaries (raw, tar.gz, zip)

### Shared Constants (catalog.h/c)

Use these instead of defining local arrays in game files:
- `PLATFORMS_LINUX[]` — `{"linux", NULL}`
- `PLATFORMS_POSIX[]` — `{"linux", "macos", NULL}`
- `PLATFORMS_ALL[]` — `{"linux", "macos", "windows", NULL}`
- `MAC_XCODE_INSTALL[]` / `MAC_XCODE_CHECK[]` — Xcode CLI tools

### Platform Filtering

- `Source.platforms` — NULL means all platforms, otherwise NULL-terminated string array
- `Game.platforms` — same semantics
- `game_supports_platform()` checks against runtime platform
- `game_matches_plat_filter()` checks against user-selected UI filter
- Use `source_cwd()` helper in main.c for computing game working directory

### Terminal vs Graphical Games

Engine strings that trigger `run_visible` (full terminal handoff): `"crossterm"`, `"ratatui"`, `"ncurses"`.
All other engines use captured `child_start` (output panel).

### UI State

- `NUM_CATEGORIES` and `NUM_PLAT_FILTERS` are `extern const int` in ui.h — use them, never hardcode counts
- Category/platform counts are cached in App struct (`cat_counts[]`, `plat_total`, `plat_installed`) — computed in `app_rebuild_filter()`, not per-frame
- Toolchain badges use sizeof-based loop, not hardcoded count

## Common Pitfalls

### Must-Do

- **Always add `default:` or handle all enum cases** in switch statements on `AcquireMethod` — the compiler warns on `-Wall` but missing a case silently falls through
- **Use shared platform constants** (`PLATFORMS_LINUX`, `PLATFORMS_POSIX`, etc.) instead of defining `static const char *platforms[]` in game files
- **Use `source_cwd()`** helper instead of inlining the `games_dir()` + `dir` pattern
- **Expose counts via ui.h** (`NUM_PLAT_FILTERS`, `NUM_CATEGORIES`) — never hardcode magic numbers for array sizes or modulo cycling
- **Cache computed values in App struct** rather than recomputing in render functions (render runs every 100ms)
- **Use `mktemp`** for temp files in shell scripts, never hardcode `/tmp/` paths (concurrency + symlink attacks)
- **Build scripts must work on both Linux and macOS**: use `nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4` for CPU count
- **Use `.shallow = 1`** for git clones unless the build specifically needs git history
- **Register new games in registry.c** — both the forward declaration AND the array entry

### Avoid

- **Don't use `sed -i`** in build scripts — BSD (macOS) and GNU have incompatible syntax. Generate files with `cat > file << EOF` instead
- **Don't duplicate cwd/cmd setup** — the terminal and graphical launch paths share common setup via `source_cwd()` and a unified cmd array
- **Don't put UI filter logic in catalog.c** — `game_matches_plat_filter()` lives in ui.c because it's UI state, while `game_supports_platform()` in catalog.c checks runtime platform
- **Don't iterate games in render functions** — use cached values from `app_rebuild_filter()`
- **Don't use `ACQUIRE_GIT` checks alone** — always pair with `ACQUIRE_DOWNLOAD` (e.g., `src->method == ACQUIRE_GIT || src->method == ACQUIRE_DOWNLOAD`) for cwd, is_installed, launch paths since both store games locally

## Data Flow

```
catalog_init() → GAMES[] populated from registry.c function pointers
app_init() → app_refresh() detects install status → app_rebuild_filter() builds filtered list
Key press → app_rebuild_filter() or begin_install/begin_uninstall
Install: begin_install() → start_{cargo,git,download}_acquire() → child_start() → poll loop
Launch: source_cwd() + cmd array → run_visible (terminal) or child_start (graphical)
```
