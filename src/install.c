#define _POSIX_C_SOURCE 200809L
#include "install.h"
#include "platform.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── LineBuf ──────────────────────────────────────────────────── */

void linebuf_init(LineBuf *lb) {
    lb->count = 0;
    lb->cap = 64;
    lb->lines = malloc((size_t)lb->cap * sizeof(char *));
}

void linebuf_push(LineBuf *lb, const char *line) {
    /* Evict oldest half when at capacity */
    if (lb->count >= LINEBUF_MAX) {
        int drop = LINEBUF_MAX / 2;
        for (int i = 0; i < drop; i++) free(lb->lines[i]);
        memmove(lb->lines, lb->lines + drop, (size_t)(lb->count - drop) * sizeof(char *));
        lb->count -= drop;
    }
    if (lb->count >= lb->cap) {
        lb->cap *= 2;
        if (lb->cap > LINEBUF_MAX) lb->cap = LINEBUF_MAX;
        lb->lines = realloc(lb->lines, (size_t)lb->cap * sizeof(char *));
    }
    lb->lines[lb->count++] = strdup(line);
}

void linebuf_free(LineBuf *lb) {
    for (int i = 0; i < lb->count; i++) free(lb->lines[i]);
    free(lb->lines);
    lb->lines = NULL;
    lb->count = lb->cap = 0;
}

/* ── Child process ────────────────────────────────────────────── */

void child_start(ChildProc *cp, const char **cmd, const char *cwd) {
    linebuf_init(&cp->output);
    cp->done = 0;
    cp->ok = 0;
    cp->partial[0] = '\0';

    /* Add initial "$ command" line */
    char cmdline[1024] = "$ ";
    for (int i = 0; cmd[i]; i++) {
        if (i > 0) strncat(cmdline, " ", sizeof(cmdline) - strlen(cmdline) - 1);
        strncat(cmdline, cmd[i], sizeof(cmdline) - strlen(cmdline) - 1);
    }
    linebuf_push(&cp->output, cmdline);
    linebuf_push(&cp->output, "");

    if (plat_proc_spawn(&cp->proc, cmd, cwd) < 0) {
        linebuf_push(&cp->output, "Error: failed to start process");
        cp->done = 1;
        cp->ok = 0;
    }
}

int child_poll(ChildProc *cp) {
    if (cp->done) return 0;

    int new_lines = 0;
    char buf[4096];

    for (;;) {
        int n = plat_proc_read(&cp->proc, buf, (int)sizeof(buf) - 1);
        if (n < 0) {
            /* EOF — pipe closed */
            /* Flush partial line */
            if (cp->partial[0]) {
                linebuf_push(&cp->output, cp->partial);
                cp->partial[0] = '\0';
                new_lines++;
            }
            /* Reap child (blocking) */
            plat_proc_join(&cp->proc, &cp->ok);
            cp->done = 1;
            break;
        }
        if (n == 0) break; /* no data available right now */

        buf[n] = '\0';
        /* Split into lines */
        char *start = buf;
        for (char *p = buf; *p; p++) {
            if (*p == '\n' || *p == '\r') {
                *p = '\0';
                /* Combine with partial */
                char full[8192];
                snprintf(full, sizeof(full), "%s%s", cp->partial, start);
                cp->partial[0] = '\0';
                linebuf_push(&cp->output, full);
                new_lines++;
                start = p + 1;
            }
        }
        /* Store remaining as partial */
        if (*start) {
            size_t plen = strlen(cp->partial);
            size_t slen = strlen(start);
            if (plen + slen < sizeof(cp->partial) - 1) {
                memcpy(cp->partial + plen, start, slen + 1);
            }
        }
    }

    /* If pipe is closed but we haven't reaped yet, check non-blocking */
    if (!cp->done) {
        int ok = 0;
        if (plat_proc_exited(&cp->proc, &ok)) {
            cp->done = 1;
            cp->ok = ok;
        }
    }

    return new_lines;
}

void child_kill(ChildProc *cp) {
    if (!cp->done) {
        plat_proc_kill(&cp->proc);
        cp->done = 1;
        cp->ok = 0;
    }
}

void child_cleanup(ChildProc *cp) {
    plat_proc_close(&cp->proc);
    linebuf_free(&cp->output);
}

/* ── Visible run ──────────────────────────────────────────────── */

/* Declared in term.c */
extern void term_restore(void);
extern void term_reenter(void);

int run_visible(const char **cmd, const char *cwd) {
    term_restore();
    int ok = plat_run_inherit(cmd, cwd);
    term_reenter();
    return ok;
}

/* ── Build commands ───────────────────────────────────────────── */

char **build_install_cmd(const Source *src) {
    if (src->method == METHOD_CARGO) {
        char **cmd = malloc(4 * sizeof(char *));
        cmd[0] = strdup("cargo");
        cmd[1] = strdup("install");
        cmd[2] = strdup(src->bin);
        cmd[3] = NULL;
        return cmd;
    }
    if (src->method == METHOD_GIT) {
        /* git-game <dir> <url> --shallow/--full [build_cmd...] */
        int cap = 16;
        char **cmd = malloc((size_t)cap * sizeof(char *));
        int n = 0;
        cmd[n++] = strdup("git-game");
        cmd[n++] = strdup(src->clone_dir);
        cmd[n++] = strdup(src->clone_url);
        cmd[n++] = strdup(src->shallow ? "--shallow" : "--full");
        if (src->build_cmd) {
            for (int i = 0; src->build_cmd[i]; i++) {
                if (n >= cap - 1) { cap *= 2; cmd = realloc(cmd, (size_t)cap * sizeof(char *)); }
                cmd[n++] = strdup(src->build_cmd[i]);
            }
        }
        cmd[n] = NULL;
        return cmd;
    }
    return NULL;
}

char **build_uninstall_cmd(const Source *src) {
    if (!src->uninstall_cmd || !src->uninstall_cmd[0]) return NULL;
    int n = 0;
    while (src->uninstall_cmd[n]) n++;
    char **cmd = malloc((size_t)(n + 1) * sizeof(char *));
    for (int i = 0; i < n; i++) cmd[i] = strdup(src->uninstall_cmd[i]);
    cmd[n] = NULL;
    return cmd;
}

void free_cmd(char **cmd) {
    if (!cmd) return;
    for (int i = 0; cmd[i]; i++) free(cmd[i]);
    free(cmd);
}

void kill_game_process(const char *bin) {
    plat_kill_by_name(bin);
}
