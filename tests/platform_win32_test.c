#include "../src/core/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int child_check(int argc, char **argv) {
    static const char *expected[] = {
        "",
        "plain",
        "with spaces",
        "embedded \"quotes\"",
        "trailing slash\\",
        "slashes\\\\before\"quote",
    };
    int expected_count = (int)(sizeof(expected) / sizeof(expected[0]));

    if (argc != expected_count + 2) return 1;
    for (int i = 0; i < expected_count; i++) {
        if (strcmp(argv[i + 2], expected[i]) != 0) return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--child") == 0)
        return child_check(argc, argv);

    const char *cmd[] = {
        argv[0],
        "--child",
        "",
        "plain",
        "with spaces",
        "embedded \"quotes\"",
        "trailing slash\\",
        "slashes\\\\before\"quote",
        NULL
    };
    if (!plat_run_silent(cmd, NULL)) {
        fprintf(stderr, "Windows command-line argument round trip failed\n");
        return 1;
    }

    const char *powershell_cmd[] = {
        "powershell", "-NoProfile", "-Command",
        "$value = @\"\nembedded \"quote\"\n\"@\n"
        "if ($value.Trim() -cne 'embedded \"quote\"') { exit 1 }",
        NULL
    };
    if (!plat_run_silent(powershell_cmd, NULL)) {
        fprintf(stderr, "PowerShell here-string round trip failed\n");
        return 1;
    }

    char *oversized = malloc(32768);
    if (!oversized) return 1;
    memset(oversized, 'x', 32767);
    oversized[32767] = '\0';
    const char *oversized_cmd[] = {argv[0], oversized, NULL};
    int oversized_started = plat_run_silent(oversized_cmd, NULL);
    free(oversized);
    if (oversized_started) {
        fprintf(stderr, "Oversized Windows command line was not rejected\n");
        return 1;
    }
    return 0;
}
