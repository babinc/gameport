#ifndef INSTALL_H
#define INSTALL_H

#include "platform.h"
#include "catalog.h"

/* ── Output line buffer (growable, capped) ────────────────────── */
#define LINEBUF_MAX 10000

typedef struct {
    char **lines;
    int count;
    int cap;
} LineBuf;

void linebuf_init(LineBuf *lb);
void linebuf_push(LineBuf *lb, const char *line);
void linebuf_free(LineBuf *lb);

/* ── Child process state ──────────────────────────────────────── */
typedef struct {
    PlatProc proc;    /* platform process handle */
    int done;         /* 1 when child has exited */
    int ok;           /* 1 if child exited successfully */
    LineBuf output;
    char partial[4096]; /* partial line buffer for pipe reads */
} ChildProc;

/* Start a command with captured output (non-blocking pipe read).
   cmd is NULL-terminated array.  cwd may be NULL. */
void child_start(ChildProc *cp, const char **cmd, const char *cwd);

/* Poll the child pipe for new output lines. Non-blocking.
   Returns number of new lines read (0 if nothing available). */
int  child_poll(ChildProc *cp);

/* Kill a running child process. */
void child_kill(ChildProc *cp);

/* Clean up child resources. */
void child_cleanup(ChildProc *cp);

/* ── High-level install commands ──────────────────────────────── */

/* Run a command visibly (restore terminal, run, re-enter TUI).
   Returns exit success (1) or failure (0). */
int  run_visible(const char **cmd, const char *cwd);

/* Build install command for a source. Returns malloc'd NULL-terminated array. */
char **build_install_cmd(const Source *src);

/* Build uninstall command. Returns malloc'd NULL-terminated array. */
char **build_uninstall_cmd(const Source *src);

/* Free a malloc'd command array. */
void free_cmd(char **cmd);

/* Kill a game process by binary name. */
void kill_game_process(const char *bin);

#endif
