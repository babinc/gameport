#define _POSIX_C_SOURCE 200809L
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

/* Suppress warn_unused_result for terminal escape writes */
#define IGNORE_RESULT(x) do { if (x) {} } while(0)

/* ── Terminal ────────────────────────────────────────────────── */

static struct termios orig_termios;
static int raw_mode_on = 0;

static void set_raw_mode(void) {
    struct termios raw = orig_termios;
    raw.c_iflag &= (unsigned)~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= (unsigned)~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= (unsigned)~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void plat_term_init(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    set_raw_mode();
    raw_mode_on = 1;
    IGNORE_RESULT(write(STDOUT_FILENO, "\033[?1049h\033[?25l", 14));
}

void plat_term_cleanup(void) {
    IGNORE_RESULT(write(STDOUT_FILENO, "\033[?25h\033[?1049l", 14));
    if (raw_mode_on) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_on = 0;
    }
}

void plat_term_suspend(void) {
    IGNORE_RESULT(write(STDOUT_FILENO, "\033[?25h\033[?1049l", 14));
    if (raw_mode_on) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
}

void plat_term_resume(void) {
    set_raw_mode();
    IGNORE_RESULT(write(STDOUT_FILENO, "\033[?1049h\033[?25l", 14));
}

void plat_term_get_size(int *w, int *h) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        *w = ws.ws_col;
        *h = ws.ws_row;
    } else {
        *w = 80;
        *h = 24;
    }
}

int plat_term_poll(int timeout_ms) {
    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN, .revents = 0 };
    return poll(&pfd, 1, timeout_ms) > 0 ? 1 : 0;
}

int plat_term_read(char *buf, int bufsize) {
    ssize_t n = read(STDIN_FILENO, buf, (size_t)bufsize);
    return n > 0 ? (int)n : 0;
}

void plat_term_write(const char *buf, size_t len) {
    if (len > 0) {
        IGNORE_RESULT(write(STDOUT_FILENO, buf, len));
    }
}

/* ── Captured process (async) ────────────────────────────────── */

static void exec_cmd(const char **cmd) {
    int n = 0;
    while (cmd[n]) n++;
    char **argv = malloc((size_t)(n + 1) * sizeof(char *));
    for (int i = 0; i < n; i++) argv[i] = (char *)cmd[i];
    argv[n] = NULL;
    execvp(argv[0], argv);
    perror(argv[0]);
    _exit(127);
}

int plat_proc_spawn(PlatProc *p, const char **cmd, const char *cwd) {
    p->pid = 0;
    p->pipe_fd = -1;

    int pipefd[2];
    if (pipe(pipefd) < 0) return -1;

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        /* Child */
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        if (cwd && chdir(cwd) != 0) {
            perror("chdir");
            _exit(127);
        }
        exec_cmd(cmd);
    }

    /* Parent */
    close(pipefd[1]);
    p->pid = pid;
    p->pipe_fd = pipefd[0];

    /* Non-blocking pipe reads */
    int flags = fcntl(p->pipe_fd, F_GETFL, 0);
    fcntl(p->pipe_fd, F_SETFL, flags | O_NONBLOCK);
    return 0;
}

int plat_proc_read(PlatProc *p, char *buf, int bufsize) {
    if (p->pipe_fd < 0) return -1;
    ssize_t n = read(p->pipe_fd, buf, (size_t)bufsize);
    if (n > 0) return (int)n;
    if (n == 0) {
        /* EOF */
        close(p->pipe_fd);
        p->pipe_fd = -1;
        return -1;
    }
    /* EAGAIN/EWOULDBLOCK — no data available */
    return 0;
}

int plat_proc_exited(PlatProc *p, int *ok) {
    if (p->pid <= 0) { *ok = 0; return 1; }
    int status;
    pid_t r = waitpid(p->pid, &status, WNOHANG);
    if (r > 0) {
        *ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
        return 1;
    }
    return 0;
}

void plat_proc_join(PlatProc *p, int *ok) {
    if (p->pid <= 0) { *ok = 0; return; }
    int status;
    waitpid(p->pid, &status, 0);
    *ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

void plat_proc_kill(PlatProc *p) {
    if (p->pid <= 0) return;
    kill(p->pid, SIGTERM);
    struct timespec ts = {0, 100000000}; /* 100ms */
    nanosleep(&ts, NULL);
    int status;
    pid_t r = waitpid(p->pid, &status, WNOHANG);
    if (r == 0) {
        kill(p->pid, SIGKILL);
        waitpid(p->pid, &status, 0);
    }
    p->pid = 0;
}

void plat_proc_close(PlatProc *p) {
    if (p->pipe_fd >= 0) {
        close(p->pipe_fd);
        p->pipe_fd = -1;
    }
    p->pid = 0;
}

/* ── Synchronous process helpers ─────────────────────────────── */

int plat_run_inherit(const char **cmd, const char *cwd) {
    pid_t pid = fork();
    if (pid == 0) {
        if (cwd && chdir(cwd) != 0) _exit(127);
        exec_cmd(cmd);
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int plat_run_silent(const char **cmd, const char *cwd) {
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open("/dev/null", O_WRONLY);
        if (fd >= 0) { dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); close(fd); }
        if (cwd && chdir(cwd) != 0) _exit(127);
        exec_cmd(cmd);
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

void plat_kill_by_name(const char *bin) {
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open("/dev/null", O_WRONLY);
        if (fd >= 0) { dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); close(fd); }
        execlp("pkill", "pkill", "-f", bin, NULL);
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
}

/* ── Filesystem ──────────────────────────────────────────────── */

int plat_file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

int plat_is_executable(const char *path) {
    return access(path, X_OK) == 0;
}

void plat_mkdir_p(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

char *plat_which(const char *bin) {
    const char *path = getenv("PATH");
    if (!path) return NULL;
    char *dup = strdup(path);
    char *saveptr = NULL;
    char *dir = strtok_r(dup, ":", &saveptr);
    while (dir) {
        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", dir, bin);
        if (access(full, X_OK) == 0) {
            free(dup);
            return strdup(full);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }
    free(dup);
    return NULL;
}

/* ── Misc ────────────────────────────────────────────────────── */

void plat_sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
