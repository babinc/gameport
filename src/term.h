#ifndef TERM_H
#define TERM_H

#include <stdint.h>

/* ── Color ────────────────────────────────────────────────────── */
typedef struct { uint8_t r, g, b; } Color;

#define CLR_NONE      ((Color){0,0,0})
#define CLR_CYAN      ((Color){0,200,200})
#define CLR_GREEN     ((Color){0,200,80})
#define CLR_RED       ((Color){200,60,60})
#define CLR_YELLOW    ((Color){220,180,60})
#define CLR_MAGENTA   ((Color){180,80,200})
#define CLR_WHITE     ((Color){220,220,230})
#define CLR_DIMWHITE  ((Color){150,150,160})
#define CLR_BORDER    ((Color){45,45,75})
#define CLR_DARK      ((Color){100,100,110})
#define CLR_DARKGRAY  ((Color){90,90,100})
#define CLR_ICON      ((Color){100,80,160})
#define CLR_BG        ((Color){18,18,28})
#define CLR_SELBG     ((Color){35,45,80})
#define CLR_BLACK     ((Color){0,0,0})
#define CLR_LABEL     ((Color){120,120,155})
#define CLR_HINT      ((Color){80,80,120})
#define CLR_SEPARATOR ((Color){40,40,65})

/* ── Cell buffer ──────────────────────────────────────────────── */
typedef struct {
    uint32_t ch;       /* Unicode codepoint */
    Color fg, bg;
    uint8_t bold;
} Cell;

typedef struct {
    Cell *cells;       /* front buffer */
    Cell *back;        /* back buffer (last drawn) */
    int w, h;
} Screen;

/* ── Terminal init / cleanup ──────────────────────────────────── */
void term_init(void);
void term_cleanup(void);
void term_get_size(int *w, int *h);

/* ── Screen buffer ────────────────────────────────────────────── */
Screen *screen_create(int w, int h);
void    screen_destroy(Screen *s);
void    screen_resize(Screen *s, int w, int h);
void    screen_clear(Screen *s, Color bg);
void    screen_flush(Screen *s);

/* ── Drawing primitives ───────────────────────────────────────── */
void scr_put(Screen *s, int x, int y, uint32_t ch, Color fg, Color bg, int bold);
void scr_str(Screen *s, int x, int y, const char *str, Color fg, Color bg, int bold);
int  scr_str_n(Screen *s, int x, int y, const char *str, int maxw, Color fg, Color bg, int bold);
void scr_hline(Screen *s, int x, int y, int w, uint32_t ch, Color fg, Color bg);
void scr_vline(Screen *s, int x, int y, int h, uint32_t ch, Color fg, Color bg);
void scr_fill(Screen *s, int x, int y, int w, int h, Color bg);

/* ── Box drawing (rounded corners) ────────────────────────────── */
void scr_box(Screen *s, int x, int y, int w, int h, Color border);
void scr_box_title(Screen *s, int x, int y, int w, const char *title, Color title_color, Color border);

/* ── Badge (inverted label with padding) ─────────────────────── */
void scr_badge(Screen *s, int x, int y, const char *text, Color fg, Color bg, int bold);

/* ── Input ────────────────────────────────────────────────────── */
enum {
    KEY_NONE = 0,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB,
    KEY_PGUP, KEY_PGDN, KEY_HOME, KEY_END,
    KEY_CHAR,   /* regular char — value in key_char */
};

typedef struct {
    int type;
    uint32_t ch;   /* for KEY_CHAR */
} KeyEvent;

/* Returns KEY_NONE if no key available (non-blocking). */
KeyEvent term_poll_key(int timeout_ms);

/* ── HSL helper ───────────────────────────────────────────────── */
Color hsl_to_rgb(double h, double s, double l);

#endif
