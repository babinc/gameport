#include "catalog.h"
#include <string.h>

/* ── Common arrays ────────────────────────────────────────────── */

const char *MAC_XCODE_INSTALL[] = {"xcode-select", "--install", NULL};
const char *MAC_XCODE_CHECK[]   = {"xcode-select", "-p", NULL};

const char *acquire_str(AcquireMethod m) {
    switch (m) {
    case ACQUIRE_CARGO:    return "cargo";
    case ACQUIRE_GIT:      return "git";
    case ACQUIRE_DOWNLOAD: return "curl";
    }
    return "unknown";
}

/* ── Platform detection ───────────────────────────────────────── */

const char *current_platform(void) {
#if defined(__linux__)
    return "linux";
#elif defined(_WIN32)
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#else
    return "unknown";
#endif
}

int current_platform_bit(void) {
#if defined(__linux__)
    return PLAT_LINUX;
#elif defined(_WIN32)
    return PLAT_WINDOWS;
#elif defined(__APPLE__)
    return PLAT_MACOS;
#else
    return 0;
#endif
}

int game_supports_platform(const Game *g) {
    return (g->platforms & current_platform_bit()) != 0;
}

const PlatformDeps *platform_deps_for_current(const Game *g) {
    const char *os = current_platform();
    for (int i = 0; i < g->num_platform_deps; i++) {
        if (strcmp(g->platform_deps[i].os, os) == 0)
            return &g->platform_deps[i];
    }
    return NULL;
}

static int source_supports_platform(const Source *s) {
    if (!s->platforms) return 1; /* 0 = not specified, inherit from game */
    return (s->platforms & current_platform_bit()) != 0;
}

const Source *default_source(const Game *g) {
    int idx = default_source_index(g);
    return idx >= 0 ? &g->sources[idx] : NULL;
}

int default_source_index(const Game *g) {
    for (int i = 0; i < g->num_sources; i++) {
        if (source_supports_platform(&g->sources[i]))
            return i;
    }
    /* Fallback to first source if nothing matches */
    return g->num_sources > 0 ? 0 : -1;
}

int count_platform_sources(const Game *g) {
    int count = 0;
    for (int i = 0; i < g->num_sources; i++) {
        if (source_supports_platform(&g->sources[i]))
            count++;
    }
    return count;
}
