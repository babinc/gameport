#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>

/* ── MSVC compatibility ──────────────────────────────────────── */
#ifdef _MSC_VER
#define strdup  _strdup
#define strtok_r strtok_s
#endif

/* ── Platform process handle ─────────────────────────────────── */

#ifdef _WIN32
#include <windows.h>
typedef struct {
    HANDLE process;
    HANDLE pipe_read;
} PlatProc;
#else
#include <sys/types.h>
typedef struct {
    pid_t pid;      /* 0 if none */
    int pipe_fd;    /* -1 if closed */
} PlatProc;
#endif

/* ── Captured process (async, piped output) ──────────────────── */

/*   Spawn a child with stdout+stderr captured via pipe.
     Returns 0 on success, -1 on error. */
int  plat_proc_spawn(PlatProc *p, const char **cmd, const char *cwd);

/*   Non-blocking read from pipe.
     Returns >0: bytes read, 0: no data, -1: EOF (pipe closed). */
int  plat_proc_read(PlatProc *p, char *buf, int bufsize);

/*   Non-blocking exit check. Returns 1 if exited, sets *ok. */
int  plat_proc_exited(PlatProc *p, int *ok);

/*   Blocking wait until exit. Sets *ok. */
void plat_proc_join(PlatProc *p, int *ok);

/*   Forcefully terminate the child process. */
void plat_proc_kill(PlatProc *p);

/*   Close handles/fds and zero the struct. */
void plat_proc_close(PlatProc *p);

/* ── Synchronous process helpers ─────────────────────────────── */

/*   Run inheriting terminal stdio (blocking). Returns 1=success. */
int  plat_run_inherit(const char **cmd, const char *cwd);

/*   Run with output suppressed (blocking). Returns 1=success. */
int  plat_run_silent(const char **cmd, const char *cwd);

/*   Kill a process by binary name. */
void plat_kill_by_name(const char *bin);

/* ── Terminal escape sequences ────────────────────────────────── */

#define TERM_ALT_SCREEN_ON  "\033[?1049h\033[?25l"  /* alt screen + hide cursor */
#define TERM_ALT_SCREEN_OFF "\033[?25h\033[?1049l"   /* show cursor + main screen */
#define TERM_ALT_SCREEN_LEN 14

/* ── Terminal ────────────────────────────────────────────────── */

void plat_term_init(void);       /* raw mode + alt screen + hide cursor */
void plat_term_cleanup(void);    /* restore mode + show cursor + main screen */
void plat_term_suspend(void);    /* temporarily leave raw/alt for visible cmds */
void plat_term_resume(void);     /* re-enter raw/alt after visible cmds */
void plat_term_get_size(int *w, int *h);
int  plat_term_poll(int timeout_ms);        /* 1 if input ready */
int  plat_term_read(char *buf, int bufsize); /* raw bytes, returns count */
void plat_term_write(const char *buf, size_t len);

/* ── Filesystem ──────────────────────────────────────────────── */

int   plat_file_exists(const char *path);
int   plat_is_executable(const char *path);
void  plat_mkdir_p(const char *path);
int   plat_rmdir_rf(const char *path);  /* recursive delete, returns 1=ok */
char *plat_which(const char *bin);   /* malloc'd path or NULL */

/* ── Misc ────────────────────────────────────────────────────── */

void plat_sleep_ms(int ms);

#endif
