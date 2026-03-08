#define _POSIX_C_SOURCE 200809L
#include "install.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>

/* ── LineBuf ──────────────────────────────────────────────────── */

void linebuf_init(LineBuf *lb) {
    lb->count = 0;
    lb->cap = 64;
    lb->lines = malloc((size_t)lb->cap * sizeof(char *));
}

void linebuf_push(LineBuf *lb, const char *line) {
    if (lb->count >= lb->cap) {
        lb->cap *= 2;
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

    /* Add initial "$ command" line */
    char cmdline[1024] = "$ ";
    for (int i = 0; cmd[i]; i++) {
        if (i > 0) strncat(cmdline, " ", sizeof(cmdline) - strlen(cmdline) - 1);
        strncat(cmdline, cmd[i], sizeof(cmdline) - strlen(cmdline) - 1);
    }
    linebuf_push(&cp->output, cmdline);
    linebuf_push(&cp->output, "");

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        linebuf_push(&cp->output, "Error: pipe() failed");
        cp->done = 1;
        cp->ok = 0;
        cp->pid = 0;
        cp->pipe_fd = -1;
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        linebuf_push(&cp->output, "Error: fork() failed");
        close(pipefd[0]);
        close(pipefd[1]);
        cp->done = 1;
        cp->ok = 0;
        cp->pid = 0;
        cp->pipe_fd = -1;
        return;
    }

    if (pid == 0) {
        /* Child */
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        if (cwd) {
            if (chdir(cwd) != 0) {
                perror("chdir");
                _exit(127);
            }
        }

        /* Count args */
        int n = 0;
        while (cmd[n]) n++;
        char **argv = malloc((size_t)(n + 1) * sizeof(char *));
        for (int i = 0; i < n; i++) argv[i] = (char *)cmd[i];
        argv[n] = NULL;
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }

    /* Parent */
    close(pipefd[1]);
    cp->pid = pid;
    cp->pipe_fd = pipefd[0];

    /* Make pipe non-blocking */
    int flags = fcntl(cp->pipe_fd, F_GETFL, 0);
    fcntl(cp->pipe_fd, F_SETFL, flags | O_NONBLOCK);
}

int child_poll(ChildProc *cp) {
    if (cp->pipe_fd < 0) {
        /* Check if child exited */
        if (cp->pid > 0 && !cp->done) {
            int status;
            pid_t r = waitpid(cp->pid, &status, WNOHANG);
            if (r > 0) {
                cp->done = 1;
                cp->ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
            }
        }
        return 0;
    }

    int new_lines = 0;
    char buf[4096];
    static char linebuf_partial[4096] = "";

    for (;;) {
        ssize_t n = read(cp->pipe_fd, buf, sizeof(buf) - 1);
        if (n <= 0) {
            if (n == 0) {
                /* EOF — pipe closed */
                /* Flush partial line */
                if (linebuf_partial[0]) {
                    linebuf_push(&cp->output, linebuf_partial);
                    linebuf_partial[0] = '\0';
                    new_lines++;
                }
                close(cp->pipe_fd);
                cp->pipe_fd = -1;

                /* Reap child */
                if (cp->pid > 0) {
                    int status;
                    waitpid(cp->pid, &status, 0);
                    cp->done = 1;
                    cp->ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
                }
            }
            break;
        }

        buf[n] = '\0';
        /* Split into lines */
        char *start = buf;
        for (char *p = buf; *p; p++) {
            if (*p == '\n' || *p == '\r') {
                *p = '\0';
                /* Combine with partial */
                char full[8192];
                snprintf(full, sizeof(full), "%s%s", linebuf_partial, start);
                linebuf_partial[0] = '\0';
                linebuf_push(&cp->output, full);
                new_lines++;
                start = p + 1;
            }
        }
        /* Store remaining as partial */
        if (*start) {
            size_t plen = strlen(linebuf_partial);
            size_t slen = strlen(start);
            if (plen + slen < sizeof(linebuf_partial) - 1) {
                memcpy(linebuf_partial + plen, start, slen + 1);
            }
        }
    }

    return new_lines;
}

void child_kill(ChildProc *cp) {
    if (cp->pid > 0 && !cp->done) {
        kill(cp->pid, SIGTERM);
        {
            struct timespec ts = {0, 100000000}; /* 100ms */
            nanosleep(&ts, NULL);
        }
        /* Force kill if still running */
        int status;
        pid_t r = waitpid(cp->pid, &status, WNOHANG);
        if (r == 0) {
            kill(cp->pid, SIGKILL);
            waitpid(cp->pid, &status, 0);
        }
        cp->done = 1;
        cp->ok = 0;
    }
    if (cp->pipe_fd >= 0) {
        close(cp->pipe_fd);
        cp->pipe_fd = -1;
    }
}

void child_cleanup(ChildProc *cp) {
    if (cp->pipe_fd >= 0) close(cp->pipe_fd);
    cp->pipe_fd = -1;
    cp->pid = 0;
    linebuf_free(&cp->output);
}

/* ── Visible run ──────────────────────────────────────────────── */

/* Declared in term.c */
extern void term_restore(void);
extern void term_reenter(void);

int run_visible(const char **cmd, const char *cwd) {
    term_restore();

    pid_t pid = fork();
    if (pid == 0) {
        if (cwd && chdir(cwd) != 0) _exit(127);
        int n = 0;
        while (cmd[n]) n++;
        char **argv = malloc((size_t)(n + 1) * sizeof(char *));
        for (int i = 0; i < n; i++) argv[i] = (char *)cmd[i];
        argv[n] = NULL;
        execvp(argv[0], argv);
        _exit(127);
    }

    int status;
    waitpid(pid, &status, 0);

    term_reenter();
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/* ── Build commands ───────────────────────────────────────────── */

char **build_install_cmd(const Source *src) {
    if (strcmp(src->method, "cargo") == 0) {
        char **cmd = malloc(4 * sizeof(char *));
        cmd[0] = strdup("cargo");
        cmd[1] = strdup("install");
        cmd[2] = strdup(src->bin);
        cmd[3] = NULL;
        return cmd;
    }
    if (strcmp(src->method, "cmake") == 0) {
        /* Return the build_cmd as-is (cmake-game commands) */
        int n = 0;
        while (src->build_cmd[n]) n++;
        char **cmd = malloc((size_t)(n + 1) * sizeof(char *));
        for (int i = 0; i < n; i++) cmd[i] = strdup(src->build_cmd[i]);
        cmd[n] = NULL;
        return cmd;
    }
    if (strcmp(src->method, "git") == 0) {
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
