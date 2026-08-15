# TUI Library — Text User Interface for fc_api

## Overview

The TUI library provides a lightweight widget toolkit for the TI-99/4A,
designed for both internal ForceCommand use and external fc_api applications.
It auto-adapts to all three supported screen modes using existing VDP/conio
globals (`displayWidth`, `displayHeight`, `nTextFlags`, `gImage`, `gColor`).

Ships with 8 widget types, modal dialog helpers, window management, focus
navigation, and a non-blocking event poll.

---

## Character Set Reference

All TUI rendering uses IBM ASCII codepage characters from the pattern table.
Applications may redefine these patterns; the TUI will render whatever glyph
is at the given codepoint.

| Hex   | Glyph | Usage                              |
|-------|-------|------------------------------------|
| `0x10`| `►`   | List selection cursor              |
| `0xB0`| `░`   | Scrollbar track                    |
| `0xB3`| `│`   | Vertical border edge               |
| `0xC0`| `└`   | Bottom-left box corner             |
| `0xC4`| `─`   | Horizontal border edge; progress empty |
| `0xD4`|       | Top-left box corner (bar style)*   |
| `0xB5`|       | Top split (bar style)*             |
| `0xC6`|       | Right split (bar style)*           |
| `0xBE`|       | Top-right corner (bar style)*      |
| `0xDA`| `┌`   | Top-left box corner                |
| `0xBF`| `┐`   | Top-right box corner               |
| `0xD9`| `┘`   | Bottom-right box corner            |
| `0xDB`| `█`   | Progress fill; scrollbar thumb     |
| `0x2D`| `-`   | Progress empty (alternative)       |
| `0x5B`| `[`   | Unfocused button left bracket      |
| `0x5D`| `]`   | Unfocused button right bracket     |
| `0x3C`| `<`   | Focused button left bracket        |
| `0x3E`| `>`   | Focused button right bracket       |
| `0x28`| `(`   | Disabled widget bracket            |
| `0x29`| `)`   | Disabled widget bracket            |
| `0x2A`| `*`   | Checkbox checked marker            |

\* Used by `drawBar` / `term_drop_down`; *not* used by TUI `tui_box`.

---

## Memory Model

The caller owns the TUI memory pool. Pass a buffer and its size to `tui_init`:

```c
char pool[2048];
tui_init(pool, sizeof(pool));
```

Pool layout:

```
pool[0 .. sizeof(tui_state) - 1]        → runtime state struct
pool[sizeof(tui_state) .. pool_size - 1] → bump-allocated widgets + strings
```

The internal bump allocator never frees individual allocations. The entire pool
is reclaimed by calling `tui_init` again (or by discard — the caller owns the
pool). `tui_done` is a no-op.

**Typical usage**: a 2 KB pool is sufficient for a window with several widgets
and a list of ~20 items. 4 KB is generous.

---

## Screen Mode Adaptation

| Query               | Underlying global       | Returns                     |
|---------------------|-------------------------|-----------------------------|
| `tui_screen_width`  | `displayWidth`          | 32, 40, or 80               |
| `tui_screen_height` | `displayHeight`         | 24, 26, or 30              |
| `tui_has_color`     | `nTextFlags & 0x8000`   | 1 on F18A 80×30, 0 otherwise |

- **32×24 Graphics mode**: borders use line-draw chars; color is global (2-color
  per 8-char block, no per-char attributes).
- **40×24 Text mode**: full box rendering; 2-color global.
- **80×24/26 Text80 mode**: full box rendering; 2-color global.
- **80×30 Text80×30 (F18A)**: full box rendering; per-character color via
  `gColor` attribute table.

The `tui_set_color` call sets `conio_scrnCol`. On F18A modes the attribute is
written per-character; on other modes it is stored but has no visible effect
(safe no-op).

---

## API Reference

### Lifecycle

```c
int  tui_init(void* pool, int pool_size);    // → 0 on success
void tui_done(void);
```

`tui_init` stores the pool address in a static pointer (in shared `lower_exp`
RAM, accessible from any bank). The pool must remain valid for the lifetime of
the TUI session.

### Screen Queries

```c
int tui_screen_width(void);
int tui_screen_height(void);
int tui_has_color(void);
```

### Drawing Primitives

Primitives operate in screen (absolute) coordinates.

```c
void tui_gotoxy(int x, int y);              // set cursor (0-based)
void tui_putc(int c);                       // write char at cursor
void tui_puts(const char* s);               // write string
void tui_set_color(int fg, int bg);         // color nibbles (0x0–0xF)
void tui_hline(int x, int y, int w);        // horizontal line (0xC4)
void tui_vline(int x, int y, int h);        // vertical line (0xB3)
void tui_box(int x, int y, int w, int h);   // bordered rectangle
void tui_box_title(int x, int y, int w, int h, const char* title);
void tui_fill(int x, int y, int w, int h, int ch);
```

`tui_box` and `tui_box_title` use single-line box-drawing characters
(`0xDA`/`0xBF`/`0xC0`/`0xD9` corners, `0xC4`/`0xB3` edges).

### Window Management

Windows are bordered rectangles. By default, the client area is the interior
(w-2 × h-2). Borders can be selectively enabled/disabled via `border_flags`:

| Flag           | Value | Meaning                                    |
|----------------|-------|--------------------------------------------|
| `TUI_BF_TOP`   | 0x01  | Title bar (top edge with corners + title)  |
| `TUI_BF_BOTTOM`| 0x02  | Footer bar (bottom edge with corners)      |
| `TUI_BF_SIDES` | 0x04  | Left + right vertical borders (paired)     |
| `TUI_BF_ALL`   | 0x07  | All borders (default)                      |

When a border is absent, that character cell becomes part of the client area.
For example, `TUI_BF_TOP | TUI_BF_BOTTOM` (no sides) gives `cx=x, cw=w`,
while `TUI_BF_TOP | TUI_BF_SIDES | TUI_BF_BOTTOM` (full) gives `cx=x+1, cw=w-2`.

```c
tui_win_t* tui_win_open(int x, int y, int w, int h);  // → NULL on OOM
void       tui_win_close(tui_win_t* win);
void       tui_win_set_title(tui_win_t* win, const char* title);
void       tui_win_set_border(tui_win_t* win, int flags);  // TUI_BF_* flags
void       tui_win_move(tui_win_t* win, int x, int y);
void       tui_win_resize(tui_win_t* win, int w, int h);
void       tui_win_set_colors(tui_win_t* win, int fg, int bg);
void       tui_win_gotoxy(tui_win_t* win, int x, int y);  // client-relative
void       tui_win_putc(tui_win_t* win, int c);
void       tui_win_puts(tui_win_t* win, const char* s);
void       tui_win_printf(tui_win_t* win, const char* fmt, ...);  // %s %d %u %%
void       tui_win_scroll(tui_win_t* win, int lines);  // +N = up, -N = down
```

`tui_win_set_border` sets the border flags and updates the client area.
To make the change visible, call `tui_win_set_title` (which redraws the border)
or trigger a window move/resize.

Windows track child widgets and focus. Closing a window destroys all children
(they are unlinked; memory is NOT freed — the pool is reset on `tui_init`).

### Base Widget Operations

```c
void tui_widget_destroy(tui_widget_t* w);
void tui_widget_show(tui_widget_t* w);
void tui_widget_hide(tui_widget_t* w);
void tui_widget_focus(tui_widget_t* w);
void tui_widget_enable(tui_widget_t* w);
void tui_widget_disable(tui_widget_t* w);
```

### Button

```c
tui_widget_t* tui_button_create(tui_win_t* win, int x, int y, int w, const char* label);
void          tui_button_set_label(tui_widget_t* btn, const char* label);
```

Rendering:
| State    | Appearance     |
|----------|----------------|
| Focused  | `< Label >`    |
| Unfocused| `[ Label ]`    |
| Disabled | `( Label )`    |

Label is centered within `w`. ENTER or SPACE activates the button (returns
`TUI_EV_WIDGET_ACTIVATED`).

### Label

```c
tui_widget_t* tui_label_create(tui_win_t* win, int x, int y, const char* text);
void          tui_label_set_text(tui_widget_t* lbl, const char* text);
```

Static text. Width is set to the string length. Labels do not receive focus and
do not handle keys.

### List

```c
tui_widget_t* tui_list_create(tui_win_t* win, int x, int y, int w, int h);
int           tui_list_add(tui_widget_t* list, const char* item);      // → index
int           tui_list_insert(tui_widget_t* list, int idx, const char* item);
void          tui_list_remove(tui_widget_t* list, int idx);
void          tui_list_clear(tui_widget_t* list);
int           tui_list_count(tui_widget_t* list);
int           tui_list_get_selected(tui_widget_t* list);               // → -1 if none
void          tui_list_set_selected(tui_widget_t* list, int idx);
```

Scrollable list box with a scrollbar when items exceed visible rows. Items are
limited to 39 characters each, 200 items maximum.

Keyboard navigation: UP/DOWN arrows, HOME/BEGIN, END, ENTER/SPACE to select.

### Text Field

```c
tui_widget_t* tui_textfield_create(tui_win_t* win, int x, int y, int w);
void          tui_textfield_set(tui_widget_t* tf, const char* text);
const char*   tui_textfield_get(tui_widget_t* tf);
void          tui_textfield_set_cursor(tui_widget_t* tf, int pos);
```

Single-line text input, max 80 characters. Content scrolls horizontally when
the cursor moves beyond the visible field width.

Keyboard: LEFT/RIGHT arrows, HOME/BEGIN, END, FCTN+1 (DELETE), FCTN+3 (ERASE),
BACK (FCTN+9 / BACKSPACE). Printable characters insert at the cursor position.

### Checkbox

```c
tui_widget_t* tui_checkbox_create(tui_win_t* win, int x, int y, const char* label);
void          tui_checkbox_set(tui_widget_t* cb, int state);  // 0 or 1
int           tui_checkbox_get(tui_widget_t* cb);
```

Rendering:
| State    | Appearance  |
|----------|-------------|
| Focused  | `<*> label` |
| Unfocused| `[*] label` |
| Disabled | `( ) label` |

ENTER or SPACE toggles the state.

### Progress Bar

```c
tui_widget_t* tui_progressbar_create(tui_win_t* win, int x, int y, int w);
void          tui_progressbar_set(tui_widget_t* pb, int value);  // 0–100
```

Rendering: `[█████-----]` using 0xDB (filled) and 0x2D (empty).

### Event System

```c
int tui_get_event(struct tui_event* ev);         // → 1 if event, 0 if none
int tui_dispatch_event(struct tui_event* ev);    // → 1 if handled
void tui_set_focus(tui_widget_t* w);
tui_widget_t* tui_get_focus(void);
```

`tui_get_event` is non-blocking. It polls `term_kscan(5)` for a key and returns
immediately. If no key is pressed, `ev->type` is `TUI_EV_NONE` and the function
returns 0.

```c
struct tui_event {
    enum tui_event_type type;   // TUI_EV_NONE, _KEY, _MOUSE_CLICK, _MOUSE_MOVE, _WIDGET_ACTIVATED
    struct tui_widget* widget;  // source widget (for WIDGET_ACTIVATED)
    int key;                    // key scancode (for KEY)
    int mx, my;                 // mouse char-cell position (for MOUSE_*)
    int buttons;                // button mask (for MOUSE_*)
};
```

Global navigation keys are handled inside `tui_get_event` before widget dispatch:
- **REDO** (6) → advance focus ring to next widget
- **BREAK** (2) → close the topmost window in the stack

### Modal Dialogs

```c
int tui_message_box(const char* title, const char* msg);     // → 0
int tui_confirm_box(const char* title, const char* msg);     // → 1 (Yes) or 0 (No)
int tui_input_box(const char* title, const char* prompt, char* buf, int len);  // → 1 (OK) or 0 (cancel)
```

Each dialog creates a centered window, runs its own modal event loop, cleans up,
and returns the result. BREAK or FCTN+4 cancels.

### Menu

```c
tui_win_t* tui_menu_create(int x, int y, const char** items, int count);  // → NULL on OOM
void       tui_menu_destroy(tui_win_t* menu);
int        tui_menu_get_selected(tui_win_t* menu);  // → index or -1
```

`tui_menu_create` builds a popup window with a list widget. The caller runs the
event loop; when a list item is activated, call `menu_get_selected` to retrieve
the chosen index, then `menu_destroy` to close.

---

## Keyboard Navigation Reference

| Key             | Code | Action                                        |
|-----------------|------|-----------------------------------------------|
| FCTN+8 (REDO)   | 6    | Next widget in focus ring                     |
| FCTN+4 (BREAK)  | 2    | Close window / cancel dialog                  |
| UP arrow        | 11   | List item up; fallback previous focus         |
| DOWN arrow      | 10   | List item down; fallback next focus           |
| LEFT arrow      | 8    | Text cursor left; previous widget (fallback)  |
| RIGHT arrow     | 9    | Text cursor right; next widget (fallback)     |
| ENTER           | 13   | Activate button, select list item             |
| SPACE           | 32   | Toggle checkbox, select list item             |
| FCTN+1 (DEL)    | 3    | Delete char at text cursor                    |
| FCTN+3 (ERASE)  | 7    | Clear text field                              |
| BACK (FCTN+9)   | 15   | Backspace (delete char before cursor)         |
| BEGIN (FCTN+5)  | 147  | List top / text field start                   |
| END             | 132  | List bottom / text field end                  |
| AID (FCTN+7)    | 1    | Application-defined help                      |

---

## Focus Ring

The focus ring order is: windows in stack order (newest first), then widget
sibling order within each window. REDO (6) advances to the next visible,
enabled widget and wraps around. UP/DOWN arrows also cycle focus as a fallback
when the current widget does not consume them.

---

## Mouse Support

Mouse polling is handled inside `tui_get_event`. Mouse is only available on
F18A hardware (per `tipi_mouse.c` limitations). Mouse pixel coordinates are
converted to character-cell coordinates based on the current font metrics.

Button 1 → focus and activate widget under pointer
Button 2 → cancel / close window

Mouse hit-testing traverses windows in reverse stack order (topmost first),
then widget children, and dispatches to the first matching widget.

---

## Bank Layout

| Bank | Contents                                  |
|------|-------------------------------------------|
| 11   | Engine: core, draw, window, widget base, input/event |
| 12   | Widgets + Dialogs + Menu                  |

Cross-bank calls use the standard `DECLARE_BANKED` / `DECLARE_BANKED_VOID`
trampolines. Widget render and handle_key functions are dispatched by type enum
in Bank 11, which then calls the Bank 12 implementation through the banked
wrapper (e.g., `bk_tui_button_render`).

---

## fc_api Integration

All 62 TUI functions have entries in `fc_api.lst` (indices 133–194). The build
system auto-generates both the assembly jump table and the SDK header
(`fcsdk/fc_api.h`). External applications include `fc_api.h` for function calls
and `tui.h` for struct/enum definitions.

---

## Example

```c
#include "fc_api.h"
#include "tui.h"

char tui_pool[2048];

int main(char* args) {
    (void)args;

    if (tui_init(tui_pool, sizeof(tui_pool)) != 0) return 1;

    tui_win_t* win = tui_win_open(10, 4, 40, 12);
    tui_win_set_title(win, "Example");

    tui_widget_t* list = tui_list_create(win, 1, 1, 28, 6);
    tui_list_add(list, "Apples");
    tui_list_add(list, "Bananas");
    tui_list_add(list, "Cherries");
    tui_list_set_selected(list, 0);

    tui_widget_t* quit_btn = tui_button_create(win, 30, 6, 8, " Quit ");

    tui_widget_t* cb = tui_checkbox_create(win, 1, 8, "Enable");
    tui_checkbox_set(cb, 1);

    tui_widget_focus(list);

    struct tui_event ev;
    int running = 1;
    while (running) {
        if (tui_get_event(&ev)) {
            tui_dispatch_event(&ev);
            if (ev.type == TUI_EV_WIDGET_ACTIVATED) {
                if (ev.widget == quit_btn) {
                    running = 0;
                } else if (ev.widget == list) {
                    int sel = tui_list_get_selected(list);
                    // use sel
                }
            }
            if (ev.type == TUI_EV_KEY && ev.key == KEY_BREAK) {
                running = 0;
            }
        }
    }

    tui_win_close(win);
    tui_done();
    return 0;
}
```

---

## Porting Notes

- The library depends on these globals (set by `setupScreen`):
  `displayWidth`, `displayHeight`, `nTextFlags`, `gImage`, `gColor`,
  `conio_scrnCol`, `conio_x`, `conio_y`.
- VDP access uses `vdpchar`, `vdpmemset`, `vdpmemcpy` — all mode-aware.
- Keyboard input uses `term_kscan(5)` via the Bank 8 DECLARE_BANKED wrapper.
- The `tui_printf` implementation supports `%s`, `%d`, `%u`, `%%` only.
- Memory: bump allocator, no individual free. Reset on `tui_init`.
