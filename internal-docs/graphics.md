# ForceCommand Graphics API

## Status

This document is the normative definition of the ForceCommand 3 high-level
graphics API. The API is C-callable and is intended for games, demos, and other
graphics applications. It is inspired by MSX and MSX2 BASIC, but it does not
use MSX screen numbers or BASIC syntax as an ABI requirement.

The API must work with the VDP capabilities detected by ForceCommand:

- TMS9918A/TMS9929A, identified as `VDP_9918`
- Yamaha V9938, identified as `VDP_9938`
- Yamaha V9958, identified as `VDP_9958`
- F18A, identified as `VDP_F18A`

F18A bitmap-layer and GPU-specific graphics are deliberately not part of this
API version. They are reserved for a follow-up project.

## Design Rules

- Every public graphics routine is prefixed with `gfx_`.
- Every operation that can fail returns `int`.
- Successful operations return `GFX_OK`, which is zero.
- Drawing routines never write outside the active mode's coordinate range.
- A failed mode change leaves the previous mode and graphics state unchanged.
- A routine requiring a bitmap, tile, text, page, or sprite mode returns an
  error when called in an incompatible mode.
- Mode and VDP capabilities are checked at runtime, not inferred by callers.
- Public parameters use `int` where practical because the banked-call ABI does
  not reliably pass 8-bit parameters.

## Status Codes

```c
#define GFX_OK                    0
#define GFX_ERR_INVALID          -1
#define GFX_ERR_UNSUPPORTED      -2
#define GFX_ERR_WRONG_MODE       -3
#define GFX_ERR_RANGE            -4
#define GFX_ERR_BUSY             -5
#define GFX_ERR_NO_VRAM          -6
```

`GFX_ERR_UNSUPPORTED` covers both an unsupported VDP capability and an
unsupported mode. Callers can inspect the current `GfxInformation` capability
bits to distinguish expected hardware differences before making a call.

Errors have no partial-success meaning. In particular, a failed mode switch
must not leave partially programmed VDP registers.

## Mode Constants

The numeric values are implementation details. Applications must use the
named constants.

```c
/* TMS9918A-compatible modes */
#define GFX_MODE_GRAPHICS1       0
#define GFX_MODE_TEXT40          1
#define GFX_MODE_GRAPHICS2       2
#define GFX_MODE_MULTICOLOR      3

/* Yamaha V9938/V9958 bitmap modes */
#define GFX_MODE_GRAPHICS3       4
#define GFX_MODE_GRAPHICS4       5
#define GFX_MODE_GRAPHICS5       6
#define GFX_MODE_GRAPHICS6       7
#define GFX_MODE_GRAPHICS7       8
#define GFX_MODE_TEXT80          9

/* V9958-only modes */
#define GFX_MODE_YJK_YAE         10
#define GFX_MODE_YJK_RGB         11
#define GFX_MODE_YJK             12

/* F18A text extension */
#define GFX_MODE_F18A_TEXT80X30  13
```

### Mode Characteristics

| Mode | Coordinate space | Colors | Supported VDPs | Primary use |
| --- | --- | ---: | --- | --- |
| `GFX_MODE_GRAPHICS1` | 32x24 tiles | 16 fixed | TMS9918A, V9938, V9958, F18A | Tile graphics and sprites |
| `GFX_MODE_TEXT40` | 40x24 characters | 16 fixed/global | TMS9918A, V9938, V9958, F18A | Text |
| `GFX_MODE_GRAPHICS2` | 256x192 pixels | 16 constrained | TMS9918A, V9938, V9958, F18A | Bitmap graphics |
| `GFX_MODE_MULTICOLOR` | 64x48 color cells | 16 fixed | TMS9918A, V9938, V9958, F18A | Low-resolution graphics |
| `GFX_MODE_GRAPHICS3` | 256x192 pixels | 16 | V9938, V9958 | Enhanced bitmap graphics |
| `GFX_MODE_GRAPHICS4` | 256x212 pixels | 16 | V9938, V9958 | Enhanced bitmap graphics |
| `GFX_MODE_GRAPHICS5` | 512x212 pixels | 4 | V9938, V9958 | Wide bitmap graphics |
| `GFX_MODE_GRAPHICS6` | 512x212 pixels | 16 | V9938, V9958 | Wide bitmap graphics |
| `GFX_MODE_GRAPHICS7` | 256x212 pixels | 256 | V9938, V9958 | 256-color graphics |
| `GFX_MODE_TEXT80` | 80x24/26 characters | 16 global | V9938, V9958, F18A | Wide text |
| `GFX_MODE_YJK_YAE` | 256x212 pixels | V9958 YJK/YAE | V9958 | V9958 color graphics |
| `GFX_MODE_YJK_RGB` | 256x212 pixels | V9958 YJK/RGB | V9958 | V9958 color graphics |
| `GFX_MODE_YJK` | 256x212 pixels | V9958 YJK | V9958 | V9958 color graphics |
| `GFX_MODE_F18A_TEXT80X30` | 80x30 characters | Per-character | F18A | F18A text |

`GFX_MODE_GRAPHICS1` is a tile mode, not a pixel framebuffer. Pixel routines
called in this mode return `GFX_ERR_WRONG_MODE`; use `gfx_pattern_define`,
`gfx_tile`, and the sprite routines instead.

`GFX_MODE_MULTICOLOR` addresses 4x4 color cells. Its coordinate range is
therefore 64x48 even though the VDP output is 256x192.

The V9938 and V9958 share the V9938 bitmap modes. V9958-only YJK modes must
return `GFX_ERR_UNSUPPORTED` on a V9938.

The F18A supports the four TMS9918A-compatible modes, the existing 40-column
and 80-column text modes, and `GFX_MODE_F18A_TEXT80X30`. It does not claim the
Yamaha bitmap or YJK modes in this API.

## Sprite Constants

```c
#define GFX_SPRITE_8X8          0x00
#define GFX_SPRITE_8X8_MAG      0x01
#define GFX_SPRITE_16X16        0x02
#define GFX_SPRITE_16X16_MAG    0x03
```

These values correspond to the TMS9918A sprite-size and magnification bits.
The `sprite_mode` argument to `gfx_screen` is ignored only for modes that do
not support sprites; passing an invalid value remains an error.

## Capability Bits

`GfxInformation.capabilities` reports operations available in the active mode.

```c
#define GFX_CAP_TEXT             0x0001
#define GFX_CAP_TILES            0x0002
#define GFX_CAP_PIXELS           0x0004
#define GFX_CAP_LINES            0x0008
#define GFX_CAP_CIRCLES          0x0010
#define GFX_CAP_PAINT            0x0020
#define GFX_CAP_COPY             0x0040
#define GFX_CAP_PAGES            0x0080
#define GFX_CAP_PALETTE          0x0100
#define GFX_CAP_SPRITES          0x0200
#define GFX_CAP_SPRITE_STATUS    0x0400
#define GFX_CAP_YJK              0x0800
#define GFX_CAP_ATTRIBUTES       0x1000
```

Capabilities describe the public behavior, not whether an operation uses a
software or hardware implementation. For example, `GFX_CAP_LINES` may be
implemented by the V9958 command engine or by a TMS9918A software routine.

## Public Structures

The following structures are part of the public SDK header.

```c
struct GfxInformation {
    int vdp_type;
    int mode;
    int width;
    int height;
    int physical_width;
    int physical_height;
    int colors;
    int color_model;
    int sprite_count;
    int display_page;
    int draw_page;
    int page_count;
    int capabilities;
    unsigned int image_addr;
    unsigned int pattern_addr;
    unsigned int color_addr;
    unsigned int sprite_addr;
    unsigned int sprite_pattern_addr;
};

struct GfxSpriteStatus {
    int collision;
    int fifth_sprite;
    int overflow;
};
```

`width` and `height` are the valid coordinates for the active API mode.
`physical_width` and `physical_height` describe the VDP output resolution.
`colors` is the maximum logical color value count, not necessarily the number
of simultaneously selectable palette entries.

`gfx_get_info` is valid after ForceCommand startup and after every successful
`gfx_screen` call. It returns `GFX_ERR_INVALID` for a null pointer.

## Color Model

### TMS9918A-Compatible Indexed Color

The color constants already exposed by the SDK remain valid:

```c
#define COLOR_TRANS       0x00
#define COLOR_BLACK       0x01
#define COLOR_MEDGREEN    0x02
#define COLOR_LTGREEN     0x03
#define COLOR_DKBLUE      0x04
#define COLOR_LTBLUE      0x05
#define COLOR_DKRED       0x06
#define COLOR_CYAN        0x07
#define COLOR_MEDRED      0x08
#define COLOR_LTRED       0x09
#define COLOR_DKYELLOW    0x0A
#define COLOR_LTYELLOW    0x0B
#define COLOR_DKGREEN     0x0C
#define COLOR_MAGENTA     0x0D
#define COLOR_GRAY        0x0E
#define COLOR_WHITE       0x0F
```

Graphics I and Graphics II use the TMS9918A color restrictions. A pixel write
in Graphics II may change the color nibble for the complete 8-pixel row group
represented by that color-table byte. The implementation must preserve the
other color nibble where possible and report `GFX_ERR_UNSUPPORTED` for an
operation that cannot preserve the requested result.

### Yamaha Indexed Color

Graphics 3 through 6 use indexed colors. The valid range is supplied by the
mode's `colors` field. Palette entries are configured with
`gfx_palette_set`.

Graphics 7 uses a direct 8-bit color value and does not use the 16-entry RGB
palette in the same way as the lower indexed modes.

### YJK Color

YJK modes accept packed 12-bit RGB colors through the macro below. Each
component is in the range 0 through 15. The implementation quantizes these
components to the V9958 YJK representation and updates the affected
four-pixel YJK group as required by the hardware. A 12-bit representation is
used because the target C ABI has 16-bit `int` values.

```c
#define GFX_RGB(r, g, b) \
    ((((r) & 0x0f) << 8) | (((g) & 0x0f) << 4) | ((b) & 0x0f))
```

YJK drawing may alter neighboring pixels because the V9958 shares chroma
components across four horizontal pixels. `gfx_point` returns the logical
12-bit RGB value produced by the current YJK group.

## Operation Constants

Logical operators are requested with these constants:

```c
#define GFX_OP_PSET              0
#define GFX_OP_PRESET            1
#define GFX_OP_AND               2
#define GFX_OP_OR                3
#define GFX_OP_XOR               4
```

`GFX_OP_PSET` replaces the destination with the requested color. `GFX_OP_PRESET`
replaces it with the requested background color. Operators not supported by a
particular color model return `GFX_ERR_UNSUPPORTED`.

Line styles are selected with:

```c
#define GFX_LINE_NORMAL          0x00
#define GFX_LINE_BOX             0x01
#define GFX_LINE_FILL            0x02
```

`GFX_LINE_BOX` draws the rectangle boundary described by the two endpoints.
`GFX_LINE_FILL` fills the rectangle described by the two endpoints. Both flags
may be combined to draw and fill the rectangle.

## Function Definitions

### Screen and State

```c
int gfx_screen(int mode, int sprite_mode, int flags);
int gfx_get_info(struct GfxInformation *info);
int gfx_color(int foreground, int background, int border);
int gfx_set_page(int display_page, int draw_page);
int gfx_set_cursor(int x, int y);
int gfx_get_cursor(int *x, int *y);
int gfx_clear(int color);
```

`gfx_screen` validates the VDP, mode, sprite mode, VRAM requirements, and
flags before changing the display. It initializes the mode tables, clears the
mode's visible data, resets the graphics cursor to `(0, 0)`, and updates
`GfxInformation`.

`flags` is reserved for mode options. The first defined flag is:

```c
#define GFX_SCREEN_INTERLACED    0x0001
```

Interlaced mode is supported only where the active Yamaha VDP and selected
mode support it. Unsupported flags return `GFX_ERR_UNSUPPORTED`.

`gfx_color` sets the current foreground, background, and border colors. It
validates each value against the active mode. A color of `-1` in drawing calls
means the current foreground or background color, as documented for that
call.

`gfx_set_page` selects the display and drawing pages for modes with page
support. It returns `GFX_ERR_UNSUPPORTED` for TMS9918A-compatible and text
modes. The two page arguments may be equal.

`gfx_clear` clears the active drawing page or visible screen using the supplied
color. `-1` selects the current background color.

### Pixels and Drawing

```c
int gfx_pset(int x, int y, int color, int op);
int gfx_preset(int x, int y, int color, int op);
int gfx_point(int x, int y, int *color);
int gfx_line(int x1, int y1, int x2, int y2,
             int color, int style, int op);
int gfx_circle(int center_x, int center_y, int radius,
               int color, int start_angle, int end_angle,
               int aspect, int op);
int gfx_paint(int x, int y, int color, int border_color, int op);
int gfx_draw(const char *commands, int color, int op);
```

`gfx_pset` writes one logical pixel or color cell. A color of `-1` selects the
current foreground color. `gfx_preset` uses the current background color when
its color argument is `-1`.

`gfx_point` reads one logical pixel or color cell into `*color`. It returns
`GFX_ERR_WRONG_MODE` when the active mode has no readable pixel representation.

`gfx_line` draws a line, rectangle boundary, or filled rectangle according to
`style`. A normal line uses the two endpoints directly. Box and fill styles
interpret the endpoints as opposite corners.

`gfx_circle` uses integer degrees for `start_angle` and `end_angle`, measured
clockwise from the positive X axis. A full circle is requested with `0` and
`360`. `aspect` is a percentage, where `100` is circular. A negative radius,
invalid angle, or non-positive aspect returns `GFX_ERR_INVALID`.

`gfx_paint` flood-fills the contiguous region containing `(x, y)` until the
specified border color is reached. It may use a software implementation on
all bitmap modes. It returns `GFX_ERR_BUSY` if the implementation cannot
obtain the temporary work area required by the selected mode.

`gfx_draw` accepts a null-terminated MSX-inspired vector command string. The
initial grammar is:

- `U`, `D`, `L`, `R`: relative vertical or horizontal movement
- `E`, `F`, `G`, `H`: relative diagonal movement
- `M x,y`: absolute move without drawing
- `B` before a movement: move without drawing
- An optional signed decimal distance after movement commands
- Spaces and commas as separators

For example, `"R40 D20 L40 U20"` draws a rectangle. Invalid command strings
return `GFX_ERR_INVALID`. The graphics cursor is updated to the final position.

All drawing operations validate coordinates before writing. The API does not
silently wrap coordinates at the screen boundary.

### Copy and Pages

```c
int gfx_copy(int source_page, int destination_page,
             int x1, int y1, int x2, int y2,
             int destination_x, int destination_y, int op);
```

`gfx_copy` copies the inclusive rectangle `(x1, y1)` through `(x2, y2)` to
`(destination_x, destination_y)` using the requested logical operator. It is
available only in bitmap modes with page support and returns
`GFX_ERR_UNSUPPORTED` otherwise.

The implementation may use the V9958 command engine. It must correctly handle
overlapping source and destination regions and must wait for command completion
before returning. Page numbers are validated against `GfxInformation.page_count`.

### Tiles and Sprites

```c
int gfx_pattern_define(int pattern, const unsigned char *data, int bytes);
int gfx_tile(int x, int y, int pattern, int color);
int gfx_sprite_pattern(int pattern, const unsigned char *data, int bytes);
int gfx_sprite(int number, int pattern, int color, int x, int y);
int gfx_sprite_loc(int number, int x, int y);
int gfx_sprite_hide(int number);
int gfx_sprite_enable(int enabled);
int gfx_sprite_status(struct GfxSpriteStatus *status);
```

`gfx_pattern_define` uploads a tile pattern. It is valid in tile modes and
accepts the pattern size required by the active mode.

`gfx_tile` places a pattern at tile coordinates and sets its color where the
active mode provides per-tile color attributes.

`gfx_sprite_pattern` uploads an 8x8 or 16x16 sprite pattern. `bytes` must match
the active sprite size and magnification rules. The API does not silently
truncate pattern data.

`gfx_sprite` sets the complete sprite attribute entry. `gfx_sprite_loc` changes
only its position. `gfx_sprite_hide` hides one sprite without changing its
pattern. `gfx_sprite_enable` enables or disables sprite display for the active
mode.

`gfx_sprite_status` reads the hardware sprite status. `collision` is nonzero
when a sprite coincidence was detected, `fifth_sprite` reports the hardware's
fifth-sprite index when available, and `overflow` reports a sprite-per-line
condition. The structure is cleared before the status is returned.

Sprite operations return `GFX_ERR_WRONG_MODE` when sprites are not active in
the current mode and `GFX_ERR_RANGE` for invalid sprite numbers or positions.

### Palette

```c
int gfx_palette_set(int index, int red, int green, int blue);
```

For F18A, components range from 0 through 15 and the existing four-bank palette
has 64 entries. TMS9918A-compatible modes use the fixed hardware palette and
return `GFX_ERR_UNSUPPORTED`. V9938/V9958 palette writes require an additional
palette data-port mapping that is not present in the current TI two-port
interface, so they also return `GFX_ERR_UNSUPPORTED` until that mapping is
provided.

Palette writes validate the active VDP and index before touching VDP registers.
The F18A implementation must unlock and relock the extended palette interface
as required by the current mode without disrupting the active text display.

## VRAM and Runtime Ownership

`gfx_screen` owns the VDP mode tables for the active graphics session. The
selected table addresses are returned by `gfx_get_info`; applications must not
assume fixed addresses.

The implementation must account for ForceCommand's existing reserved VRAM,
including command/history storage and the F18A GPU scroll area. ForceCommand
filesystem calls also use VDP-resident PAB, parameter, and file-data buffers.
Those buffers must be relocated and reserved as part of each successful mode
change; applications must use the addresses reported by the existing system
information API rather than assuming fixed addresses. A mode that cannot fit
its required tables and runtime buffers without conflict returns
`GFX_ERR_NO_VRAM`.

The API must update the existing VDP state used by terminal and TUI code,
including `gImage`, `gColor`, `gPattern`, `gSprite`, `gSpritePat`,
`gUnblank`, and the text flags where applicable. Existing `vdp_screenmode` and
terminal APIs remain available as lower-level/legacy interfaces.

VDP address and data-port sequences must be atomic. Graphics routines must
prevent interrupt or nested-call interleaving during multi-byte VDP
transactions. V9958 command routines must not return while the command engine
is busy.

## ForceCommand API Integration

The public declarations belong in `src/libti99/graphics.h` and must use the
existing `DECLARE_BANKED` or `DECLARE_BANKED_VOID` conventions. Since every
operation returns a status, the non-void banked declaration is normally used.

The implementation should be placed in a bank with sufficient room, currently
bank 14 rather than bank 8. The following integration files are required:

- `src/libti99/graphics.h`
- `src/libti99/graphics.c` and any backend files
- `src/libti99/vdp.h` for shared VDP definitions
- `fc_api.lst`
- `fc_api_template` for public constants and structures
- Generated `example/gcc/fcsdk/fc_api.h`

API entries may be reordered because the ForceCommand 3 ABI has not been
released. The API list should nevertheless remain grouped by subsystem and
the generated header must be treated as the SDK output of the build process.

## Required Demonstration and Verification

The graphics demo must demonstrate:

- Named mode selection and capability reporting
- Pixel, line, rectangle, circle, paint, draw, and copy operations
- Tile and sprite operations
- Palette and page operations where supported
- Expected errors for unsupported VDPs and wrong screen modes
- V9938 rejection of V9958-only YJK modes
- F18A text and per-character attribute behavior

Verification must include both cartridge and console-ROM builds with
`-Werror`, generated API inspection, linker map size checks, emulator testing,
and hardware testing on the available TMS9918A, V9938, V9958, and F18A systems.
