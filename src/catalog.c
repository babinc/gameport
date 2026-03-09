#include "catalog.h"
#include <string.h>

const char *acquire_str(AcquireMethod m) {
    switch (m) {
    case ACQUIRE_CARGO: return "cargo";
    case ACQUIRE_GIT:   return "git";
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

int game_supports_platform(const Game *g) {
    if (!g->platforms) return 1;
    const char *plat = current_platform();
    for (int i = 0; g->platforms[i]; i++) {
        if (strcmp(g->platforms[i], plat) == 0) return 1;
    }
    return 0;
}

const PlatformDeps *platform_deps_for_current(const Game *g) {
    const char *os = current_platform();
    for (int i = 0; i < g->num_deps; i++) {
        if (strcmp(g->platform_deps[i].os, os) == 0)
            return &g->platform_deps[i];
    }
    return NULL;
}

static int source_supports_platform(const Source *s) {
    if (!s->platforms) return 1;
    const char *plat = current_platform();
    for (int i = 0; s->platforms[i]; i++) {
        if (strcmp(s->platforms[i], plat) == 0) return 1;
    }
    return 0;
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
