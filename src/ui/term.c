#include "term.h"
#include "../core/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ── Terminal init / cleanup ──────────────────────────────────── */

void term_init(void)    { plat_term_init(); }
void term_cleanup(void) { plat_term_cleanup(); }

void term_get_size(int *w, int *h) { plat_term_get_size(w, h); }

/* Used by run_visible in install.c */
void term_restore(void) { plat_term_suspend(); }
void term_reenter(void) { plat_term_resume(); }

/* ── Screen buffer ────────────────────────────────────────────── */

Screen *screen_create(int w, int h) {
    Screen *s = calloc(1, sizeof(Screen));
    s->w = w;
    s->h = h;
    s->cells = calloc((size_t)(w * h), sizeof(Cell));
    s->back  = calloc((size_t)(w * h), sizeof(Cell));
    /* Invalidate back buffer so first flush draws everything */
    for (int i = 0; i < w * h; i++) {
        s->back[i].ch = 0xFFFFFFFF;
    }
    return s;
}

void screen_destroy(Screen *s) {
    if (!s) return;
    free(s->cells);
    free(s->back);
    free(s);
}

void screen_resize(Screen *s, int w, int h) {
    free(s->cells);
    free(s->back);
    s->w = w;
    s->h = h;
    s->cells = calloc((size_t)(w * h), sizeof(Cell));
    s->back  = calloc((size_t)(w * h), sizeof(Cell));
    for (int i = 0; i < w * h; i++) {
        s->back[i].ch = 0xFFFFFFFF;
    }
}

void screen_clear(Screen *s, Color bg) {
    for (int i = 0; i < s->w * s->h; i++) {
        s->cells[i].ch = ' ';
        s->cells[i].fg = CLR_WHITE;
        s->cells[i].bg = bg;
        s->cells[i].bold = 0;
    }
}

/* Encode a UTF-8 codepoint into buf, return number of bytes */
static int utf8_encode(uint32_t cp, char *buf) {
    if (cp < 0x80) {
        buf[0] = (char)cp;
        return 1;
    } else if (cp < 0x800) {
        buf[0] = (char)(0xC0 | (cp >> 6));
        buf[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp < 0x10000) {
        buf[0] = (char)(0xE0 | (cp >> 12));
        buf[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else {
        buf[0] = (char)(0xF0 | (cp >> 18));
        buf[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
}

void screen_flush(Screen *s) {
    /* Build output in a buffer to minimize write calls */
    size_t cap = (size_t)(s->w * s->h) * 40;
    char *buf = malloc(cap);
    size_t pos = 0;
    int last_r = -1, last_c = -1;
    Color last_fg = {255,255,255}, last_bg = {0,0,0};
    int last_bold = -1;

    #define EMIT(str, len) do { \
        if (pos + (size_t)(len) > cap) { cap *= 2; buf = realloc(buf, cap); } \
        memcpy(buf + pos, str, (size_t)(len)); pos += (size_t)(len); \
    } while(0)

    for (int r = 0; r < s->h; r++) {
        for (int c = 0; c < s->w; c++) {
            int idx = r * s->w + c;
            Cell *cell = &s->cells[idx];
            Cell *prev = &s->back[idx];
            if (cell->ch == prev->ch &&
                cell->fg.r == prev->fg.r && cell->fg.g == prev->fg.g && cell->fg.b == prev->fg.b &&
                cell->bg.r == prev->bg.r && cell->bg.g == prev->bg.g && cell->bg.b == prev->bg.b &&
                cell->bold == prev->bold) {
                continue;
            }

            /* Move cursor if not sequential */
            if (r != last_r || c != last_c) {
                char mv[24];
                int n = snprintf(mv, sizeof(mv), "\033[%d;%dH", r + 1, c + 1);
                EMIT(mv, n);
            }

            /* Set attributes */
            if (cell->bold != last_bold) {
                if (cell->bold) { EMIT("\033[1m", 4); }
                else            { EMIT("\033[22m", 5); }
                last_bold = cell->bold;
            }
            if (cell->fg.r != last_fg.r || cell->fg.g != last_fg.g || cell->fg.b != last_fg.b) {
                char seq[24];
                int n = snprintf(seq, sizeof(seq), "\033[38;2;%d;%d;%dm",
                                 cell->fg.r, cell->fg.g, cell->fg.b);
                EMIT(seq, n);
                last_fg = cell->fg;
            }
            if (cell->bg.r != last_bg.r || cell->bg.g != last_bg.g || cell->bg.b != last_bg.b) {
                char seq[24];
                int n = snprintf(seq, sizeof(seq), "\033[48;2;%d;%d;%dm",
                                 cell->bg.r, cell->bg.g, cell->bg.b);
                EMIT(seq, n);
                last_bg = cell->bg;
            }

            /* Write character */
            char u[4];
            uint32_t out_ch = cell->ch;
            if (out_ch < 0x20 || out_ch == 0x7F) out_ch = ' ';
            int ulen = utf8_encode(out_ch, u);
            EMIT(u, ulen);

            last_r = r;
            last_c = c + 1;
        }
    }
    #undef EMIT

    if (pos > 0) {
        plat_term_write(buf, pos);
    }
    free(buf);

    /* Swap buffers */
    memcpy(s->back, s->cells, (size_t)(s->w * s->h) * sizeof(Cell));
}

/* ── Drawing primitives ───────────────────────────────────────── */

void scr_put(Screen *s, int x, int y, uint32_t ch, Color fg, Color bg, int bold) {
    if (x < 0 || x >= s->w || y < 0 || y >= s->h) return;
    Cell *c = &s->cells[y * s->w + x];
    c->ch = ch;
    c->fg = fg;
    c->bg = bg;
    c->bold = (uint8_t)bold;
}

/* Decode one UTF-8 codepoint from str, advance *str, return codepoint */
static uint32_t utf8_decode(const char **str) {
    const unsigned char *s = (const unsigned char *)*str;
    uint32_t cp;
    int len;
    if (s[0] < 0x80)       { cp = s[0]; len = 1; }
    else if (s[0] < 0xE0)  { cp = s[0] & 0x1F; len = 2; }
    else if (s[0] < 0xF0)  { cp = s[0] & 0x0F; len = 3; }
    else                    { cp = s[0] & 0x07; len = 4; }
    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) break;
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    *str = (const char *)(s + len);
    return cp;
}

void scr_str(Screen *s, int x, int y, const char *str, Color fg, Color bg, int bold) {
    while (*str) {
        uint32_t cp = utf8_decode(&str);
        scr_put(s, x++, y, cp, fg, bg, bold);
    }
}

int scr_str_n(Screen *s, int x, int y, const char *str, int maxw, Color fg, Color bg, int bold) {
    int drawn = 0;
    while (*str && drawn < maxw) {
        uint32_t cp = utf8_decode(&str);
        scr_put(s, x++, y, cp, fg, bg, bold);
        drawn++;
    }
    return drawn;
}

void scr_hline(Screen *s, int x, int y, int w, uint32_t ch, Color fg, Color bg) {
    for (int i = 0; i < w; i++) scr_put(s, x + i, y, ch, fg, bg, 0);
}

void scr_vline(Screen *s, int x, int y, int h, uint32_t ch, Color fg, Color bg) {
    for (int i = 0; i < h; i++) scr_put(s, x, y + i, ch, fg, bg, 0);
}

void scr_fill(Screen *s, int x, int y, int w, int h, Color bg) {
    for (int r = 0; r < h; r++)
        for (int c = 0; c < w; c++)
            scr_put(s, x + c, y + r, ' ', CLR_WHITE, bg, 0);
}

/* ── Box drawing (rounded corners like ratatui) ───────────────── */

void scr_box(Screen *s, int x, int y, int w, int h, Color border) {
    /* Rounded corners: ╭ ╮ ╰ ╯ */
    scr_put(s, x,         y,         0x256D, border, CLR_BG, 0); /* ╭ */
    scr_put(s, x + w - 1, y,         0x256E, border, CLR_BG, 0); /* ╮ */
    scr_put(s, x,         y + h - 1, 0x2570, border, CLR_BG, 0); /* ╰ */
    scr_put(s, x + w - 1, y + h - 1, 0x256F, border, CLR_BG, 0); /* ╯ */
    /* Horizontal: ─ */
    scr_hline(s, x + 1, y,         w - 2, 0x2500, border, CLR_BG);
    scr_hline(s, x + 1, y + h - 1, w - 2, 0x2500, border, CLR_BG);
    /* Vertical: │ */
    scr_vline(s, x,         y + 1, h - 2, 0x2502, border, CLR_BG);
    scr_vline(s, x + w - 1, y + 1, h - 2, 0x2502, border, CLR_BG);
}

void scr_box_title(Screen *s, int x, int y, int w, const char *title, Color title_color, Color border) {
    (void)w; (void)border;
    /* Draw " [ TITLE ] " on top border */
    int tx = x + 2;
    scr_str(s, tx, y, " [ ", CLR_DARKGRAY, CLR_BG, 0);
    tx += 3;
    scr_str(s, tx, y, title, title_color, CLR_BG, 1);
    tx += (int)strlen(title);
    scr_str(s, tx, y, " ] ", CLR_DARKGRAY, CLR_BG, 0);
}

/* ── Badge (inverted label with padding) ─────────────────────── */

void scr_badge(Screen *s, int x, int y, const char *text, Color fg, Color bg, int bold) {
    scr_put(s, x, y, ' ', fg, bg, bold);
    int cx = x + 1;
    while (*text) {
        uint32_t cp = utf8_decode(&text);
        scr_put(s, cx++, y, cp, fg, bg, bold);
    }
    scr_put(s, cx, y, ' ', fg, bg, bold);
}

/* ── Input ────────────────────────────────────────────────────── */

KeyEvent term_poll_key(int timeout_ms) {
    KeyEvent ev = {KEY_NONE, 0};
    if (!plat_term_poll(timeout_ms)) return ev;

    unsigned char buf[8];
    int n = plat_term_read((char *)buf, (int)sizeof(buf));
    if (n <= 0) return ev;

    if (buf[0] == 27) {
        if (n == 1) {
            ev.type = KEY_ESC;
        } else if (buf[1] == '[') {
            switch (buf[2]) {
                case 'A': ev.type = KEY_UP;    break;
                case 'B': ev.type = KEY_DOWN;  break;
                case 'C': ev.type = KEY_RIGHT; break;
                case 'D': ev.type = KEY_LEFT;  break;
                case 'H': ev.type = KEY_HOME;  break;
                case 'F': ev.type = KEY_END;   break;
                case '5': if (n > 3 && buf[3] == '~') ev.type = KEY_PGUP;  break;
                case '6': if (n > 3 && buf[3] == '~') ev.type = KEY_PGDN;  break;
                default: break;
            }
        }
    } else if (buf[0] == '\r' || buf[0] == '\n') {
        ev.type = KEY_ENTER;
    } else if (buf[0] == 127 || buf[0] == 8) {
        ev.type = KEY_BACKSPACE;
    } else if (buf[0] == '\t') {
        ev.type = KEY_TAB;
    } else if (buf[0] >= 32) {
        ev.type = KEY_CHAR;
        /* Decode UTF-8 */
        const char *p = (const char *)buf;
        ev.ch = utf8_decode(&p);
    }
    return ev;
}

/* ── HSL helper ───────────────────────────────────────────────── */

Color hsl_to_rgb(double h, double s, double l) {
    double c = (1.0 - fabs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c / 2.0;
    double r, g, b;
    if      (h < 60)  { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }
    Color clr;
    clr.r = (uint8_t)((r + m) * 255.0);
    clr.g = (uint8_t)((g + m) * 255.0);
    clr.b = (uint8_t)((b + m) * 255.0);
    return clr;
}
