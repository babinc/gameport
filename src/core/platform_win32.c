#ifdef _WIN32

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>     /* _mkdir */
#include <io.h>         /* _access */
#include <windows.h>

/* ── Helpers ─────────────────────────────────────────────────── */

enum { WINDOWS_CMDLINE_MAX = 32767 };

static int cmdline_putc(char *out, size_t outlen, size_t *pos, char ch) {
    if (*pos + 1 >= outlen) return 0;
    out[(*pos)++] = ch;
    out[*pos] = '\0';
    return 1;
}

/* Quote one argv element using the parsing rules used by Windows C runtimes.
   In particular, quotes inside PowerShell -Command scripts must survive
   CreateProcess and reach the child as literal quote characters. */
static int cmdline_append_arg(char *out, size_t outlen, size_t *pos,
                              const char *arg) {
    int needs_quote = !arg[0] || strpbrk(arg, " \t\n\v\"") != NULL;
    if (!needs_quote) {
        while (*arg) {
            if (!cmdline_putc(out, outlen, pos, *arg++)) return 0;
        }
        return 1;
    }

    if (!cmdline_putc(out, outlen, pos, '"')) return 0;
    while (*arg) {
        size_t backslashes = 0;
        while (*arg == '\\') {
            backslashes++;
            arg++;
        }

        if (*arg == '"') {
            /* Double preceding backslashes, then escape the quote. */
            for (size_t i = 0; i < backslashes * 2 + 1; i++)
                if (!cmdline_putc(out, outlen, pos, '\\')) return 0;
            if (!cmdline_putc(out, outlen, pos, *arg++)) return 0;
        } else {
            /* Backslashes only need doubling immediately before the closing
               quote, where they would otherwise escape it. */
            size_t count = *arg ? backslashes : backslashes * 2;
            for (size_t i = 0; i < count; i++)
                if (!cmdline_putc(out, outlen, pos, '\\')) return 0;
            if (*arg && !cmdline_putc(out, outlen, pos, *arg++)) return 0;
        }
    }
    return cmdline_putc(out, outlen, pos, '"');
}

/* Build a flat command line from a NULL-terminated argv. The Windows limit
   includes the terminating NUL, so reject commands that cannot fit. */
static char *build_cmdline(const char **cmd) {
    size_t pos = 0;
    char *out = malloc(WINDOWS_CMDLINE_MAX);
    if (!out) return NULL;
    out[0] = '\0';
    for (int i = 0; cmd[i]; i++) {
        if ((i > 0 && !cmdline_putc(out, WINDOWS_CMDLINE_MAX, &pos, ' ')) ||
            !cmdline_append_arg(out, WINDOWS_CMDLINE_MAX, &pos, cmd[i])) {
            free(out);
            return NULL;
        }
    }
    return out;
}

/* ── Terminal ────────────────────────────────────────────────── */

static DWORD orig_in_mode;
static DWORD orig_out_mode;
static UINT  orig_out_cp;
static UINT  orig_in_cp;
static int   raw_mode_on = 0;

/* Flag values that may be missing from older SDK headers */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif
#ifndef DISABLE_NEWLINE_AUTO_RETURN
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
#endif

static void set_raw_mode(void) {
    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Input: VT input sequences, no line editing, no echo */
    DWORD in_mode = ENABLE_VIRTUAL_TERMINAL_INPUT;
    SetConsoleMode(hIn, in_mode);

    /* Output: enable ANSI/VT escape processing */
    DWORD out_mode = orig_out_mode
                   | ENABLE_VIRTUAL_TERMINAL_PROCESSING
                   | DISABLE_NEWLINE_AUTO_RETURN;
    SetConsoleMode(hOut, out_mode);
}

void plat_term_init(void) {
    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleMode(hIn, &orig_in_mode);
    GetConsoleMode(hOut, &orig_out_mode);
    orig_out_cp = GetConsoleOutputCP();
    orig_in_cp  = GetConsoleCP();

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    set_raw_mode();
    raw_mode_on = 1;

    /* Alternate screen + hide cursor */
    plat_term_write(TERM_ALT_SCREEN_ON, TERM_ALT_SCREEN_ON_LEN);
}

void plat_term_cleanup(void) {
    /* Show cursor + leave alternate screen */
    plat_term_write(TERM_ALT_SCREEN_OFF, TERM_ALT_SCREEN_OFF_LEN);

    if (raw_mode_on) {
        HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleMode(hIn, orig_in_mode);
        SetConsoleMode(hOut, orig_out_mode);
        SetConsoleOutputCP(orig_out_cp);
        SetConsoleCP(orig_in_cp);
        raw_mode_on = 0;
    }
}

void plat_term_suspend(void) {
    plat_term_write(TERM_ALT_SCREEN_OFF, TERM_ALT_SCREEN_OFF_LEN);
    if (raw_mode_on) {
        HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleMode(hIn, orig_in_mode);
        SetConsoleMode(hOut, orig_out_mode);
    }
}

void plat_term_resume(void) {
    set_raw_mode();
    plat_term_write(TERM_ALT_SCREEN_ON "\033[2J", TERM_ALT_SCREEN_ON_LEN + 4);
}

void plat_term_get_size(int *w, int *h) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        *w = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
        *h = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
    } else {
        *w = 80;
        *h = 24;
    }
}

int plat_term_poll(int timeout_ms) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD result = WaitForSingleObject(hIn, (DWORD)(timeout_ms < 0 ? INFINITE : (unsigned)timeout_ms));
    if (result != WAIT_OBJECT_0) return 0;

    /* Filter out non-key events that wake WaitForSingleObject */
    DWORD count = 0;
    GetNumberOfConsoleInputEvents(hIn, &count);
    if (count == 0) return 0;

    /* Peek to check if there's real input (VT mode should only give key events,
       but window resize / mouse events may sneak in) */
    INPUT_RECORD rec;
    DWORD peeked = 0;
    while (PeekConsoleInputW(hIn, &rec, 1, &peeked) && peeked > 0) {
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown)
            return 1;
        /* Consume non-key events */
        ReadConsoleInputW(hIn, &rec, 1, &peeked);
    }
    return 0;
}

int plat_term_read(char *buf, int bufsize) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD bytesRead = 0;
    /* With ENABLE_VIRTUAL_TERMINAL_INPUT, ReadConsoleA gives us VT sequences */
    if (!ReadConsoleA(hIn, buf, (DWORD)bufsize, &bytesRead, NULL))
        return 0;
    return (int)bytesRead;
}

void plat_term_write(const char *buf, size_t len) {
    if (len == 0) return;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written = 0;
    WriteFile(hOut, buf, (DWORD)len, &written, NULL);
}

/* ── Captured process (async) ────────────────────────────────── */

int plat_proc_spawn(PlatProc *p, const char **cmd, const char *cwd) {
    p->process = NULL;
    p->pipe_read = NULL;
    p->job = NULL;

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE pipe_read, pipe_write;
    if (!CreatePipe(&pipe_read, &pipe_write, &sa, 0))
        return -1;

    /* Prevent child from inheriting the read end */
    SetHandleInformation(pipe_read, HANDLE_FLAG_INHERIT, 0);

    /* Build command line */
    char *cmdline = build_cmdline(cmd);
    if (!cmdline) {
        CloseHandle(pipe_read);
        CloseHandle(pipe_write);
        return -1;
    }

    STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = pipe_write;
    si.hStdError  = pipe_write;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    BOOL ok = CreateProcessA(
        NULL, cmdline, NULL, NULL, TRUE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, cwd,
        &si, &pi);

    free(cmdline);
    CloseHandle(pipe_write);

    if (!ok) {
        CloseHandle(pipe_read);
        return -1;
    }

    /* A job lets cancellation terminate the complete operation tree (for
       example PowerShell -> cmake -> MSBuild), not only its immediate parent. */
    HANDLE job = CreateJobObjectA(NULL, NULL);
    if (job && !AssignProcessToJobObject(job, pi.hProcess)) {
        CloseHandle(job);
        job = NULL;
    }
    if (ResumeThread(pi.hThread) == (DWORD)-1) {
        if (job) TerminateJobObject(job, 1);
        else TerminateProcess(pi.hProcess, 1);
        if (job) CloseHandle(job);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(pipe_read);
        return -1;
    }

    CloseHandle(pi.hThread);
    p->process   = pi.hProcess;
    p->pipe_read = pipe_read;
    p->job       = job;
    return 0;
}

int plat_proc_read(PlatProc *p, char *buf, int bufsize) {
    if (!p->pipe_read) return -1;

    DWORD available = 0;
    if (!PeekNamedPipe(p->pipe_read, NULL, 0, NULL, &available, NULL)) {
        /* Pipe broken — child closed its end */
        CloseHandle(p->pipe_read);
        p->pipe_read = NULL;
        return -1;
    }
    if (available == 0) return 0;

    DWORD bytesRead = 0;
    DWORD toRead = (DWORD)bufsize < available ? (DWORD)bufsize : available;
    if (!ReadFile(p->pipe_read, buf, toRead, &bytesRead, NULL)) {
        CloseHandle(p->pipe_read);
        p->pipe_read = NULL;
        return -1;
    }
    return (int)bytesRead;
}

int plat_proc_exited(PlatProc *p, int *ok) {
    if (!p->process) { *ok = 0; return 1; }
    DWORD result = WaitForSingleObject(p->process, 0);
    if (result == WAIT_OBJECT_0) {
        DWORD exitCode = 1;
        GetExitCodeProcess(p->process, &exitCode);
        *ok = (exitCode == 0) ? 1 : 0;
        return 1;
    }
    return 0;
}

void plat_proc_join(PlatProc *p, int *ok) {
    if (!p->process) { *ok = 0; return; }
    WaitForSingleObject(p->process, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(p->process, &exitCode);
    *ok = (exitCode == 0) ? 1 : 0;
}

void plat_proc_kill(PlatProc *p) {
    if (!p->process) return;
    if (p->job) TerminateJobObject(p->job, 1);
    else TerminateProcess(p->process, 1);
    WaitForSingleObject(p->process, 3000); /* wait up to 3s for termination */
}

void plat_proc_close(PlatProc *p) {
    if (p->pipe_read) {
        CloseHandle(p->pipe_read);
        p->pipe_read = NULL;
    }
    if (p->process) {
        CloseHandle(p->process);
        p->process = NULL;
    }
    if (p->job) {
        CloseHandle(p->job);
        p->job = NULL;
    }
}

/* ── Synchronous process helpers ─────────────────────────────── */

int plat_run_inherit(const char **cmd, const char *cwd) {
    char *cmdline = build_cmdline(cmd);
    if (!cmdline) return 0;

    STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, cwd, &si, &pi)) {
        free(cmdline);
        return 0;
    }
    free(cmdline);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exitCode == 0 ? 1 : 0;
}

int plat_run_silent(const char **cmd, const char *cwd) {
    char *cmdline = build_cmdline(cmd);
    if (!cmdline) return 0;

    /* Open NUL device for output suppression */
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE nul = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_WRITE,
                             &sa, OPEN_EXISTING, 0, NULL);

    STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = nul;
    si.hStdError  = nul;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    BOOL ok = CreateProcessA(NULL, cmdline, NULL, NULL, TRUE,
                             CREATE_NO_WINDOW, NULL, cwd, &si, &pi);
    free(cmdline);
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);

    if (!ok) return 0;

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exitCode == 0 ? 1 : 0;
}

/* ── Filesystem ──────────────────────────────────────────────── */

int plat_file_exists(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES;
}

int plat_is_executable(const char *path) {
    /* On Windows, "executable" means the file exists and has an
       executable extension. For simplicity, just check existence. */
    return plat_file_exists(path);
}

void plat_mkdir_p(const char *path) {
    char tmp[MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", path);
    /* Normalize to backslash */
    for (char *c = tmp; *c; c++) {
        if (*c == '/') *c = '\\';
    }
    for (char *c = tmp + 1; *c; c++) {
        if (*c == '\\') {
            *c = '\0';
            _mkdir(tmp);
            *c = '\\';
        }
    }
    _mkdir(tmp);
}

static int rmdir_rf_impl(const char *path) {
    char search[MAX_PATH];
    snprintf(search, sizeof(search), "%s\\*", path);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(search, &fd);
    if (h == INVALID_HANDLE_VALUE) return DeleteFileA(path);
    int ok = 1;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;
        char child[MAX_PATH];
        snprintf(child, sizeof(child), "%s\\%s", path, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            ok = rmdir_rf_impl(child) && ok;
        else
            ok = DeleteFileA(child) && ok;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return RemoveDirectoryA(path) && ok;
}

int plat_rmdir_rf(const char *path) {
    return rmdir_rf_impl(path);
}

char *plat_which(const char *bin) {
    const char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    const char *pathext = getenv("PATHEXT");
    if (!pathext) pathext = ".COM;.EXE;.BAT;.CMD";

    /* Pre-split PATHEXT once to avoid re-allocating per directory */
    char *ext_dup = strdup(pathext);
    const char *exts[32];
    int next = 0;
    {
        char *save = NULL;
        char *tok = strtok_r(ext_dup, ";", &save);
        while (tok && next < 31) {
            exts[next++] = tok;
            tok = strtok_r(NULL, ";", &save);
        }
    }

    char *path_dup = strdup(path_env);
    char *save1 = NULL;
    char *dir = strtok_r(path_dup, ";", &save1);

    while (dir) {
        char full[MAX_PATH];

        /* Try exact name first */
        snprintf(full, sizeof(full), "%s\\%s", dir, bin);
        if (GetFileAttributesA(full) != INVALID_FILE_ATTRIBUTES) {
            free(ext_dup);
            free(path_dup);
            return strdup(full);
        }

        /* Try with each PATHEXT extension */
        for (int i = 0; i < next; i++) {
            snprintf(full, sizeof(full), "%s\\%s%s", dir, bin, exts[i]);
            if (GetFileAttributesA(full) != INVALID_FILE_ATTRIBUTES) {
                free(ext_dup);
                free(path_dup);
                return strdup(full);
            }
        }
        dir = strtok_r(NULL, ";", &save1);
    }
    free(ext_dup);
    free(path_dup);
    return NULL;
}

long long plat_dir_size(const char *path) {
    WIN32_FIND_DATAA fd;
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return -1;
    long long total = 0;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;
        char child[MAX_PATH];
        snprintf(child, sizeof(child), "%s\\%s", path, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            total += plat_dir_size(child);
        else
            total += ((long long)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return total;
}

static char *find_exec_impl(const char *dir, const char *name, int depth) {
    if (depth > 4) return NULL;
    WIN32_FIND_DATAA fd;
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return NULL;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;
        char child[MAX_PATH];
        snprintf(child, sizeof(child), "%s\\%s", dir, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char *found = find_exec_impl(child, name, depth + 1);
            if (found) { FindClose(h); return found; }
        } else if (_stricmp(fd.cFileName, name) == 0) {
            FindClose(h);
            return strdup(child);
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return NULL;
}

char *plat_find_executable(const char *dir, const char *name) {
    return find_exec_impl(dir, name, 0);
}

/* ── Misc ────────────────────────────────────────────────────── */

void plat_sleep_ms(int ms) {
    Sleep((DWORD)ms);
}

void plat_open_url(const char *url) {
    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

#endif /* _WIN32 */
