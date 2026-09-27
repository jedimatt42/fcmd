# GFXBITMAP - minimal graphics transition probe

Small debugging aid: walk a list of graphics modes, returning to TEXT40 between
each, then return. The header is not screen-safe, so ForceCommand restores a
text mode when this program returns.

```
GFXBITMAP <modes> [flags...]
```

Flags (may be mixed in anywhere):
- `n` — do not draw anything in the graphics modes
- `r` — do not return to TEXT40 explicitly at the end (let ForceCommand do it)
- `w` — wait ~180 VDP frames in each graphics mode (mimics GFXMODES)
- `k` — after returning to TEXT40, wait for a key (console KSCAN, like GFXMODES)
- `s` — simple drawing (two diagonals); default matches `GFXMODES` `show_demo` (adds a box and a circle)
- `b` — draw the box but not the circle
- `o` — draw the circle but not the box

Mode letters:
- `t` TEXT40, `m` MULTICOLOR, `8` TEXT80
- `1` GRAPHICS1, `2` GRAPHICS2, `3` GRAPHICS3, `4` GRAPHICS4
- `5` GRAPHICS5, `6` GRAPHICS6, `7` GRAPHICS7
- `y` YJK, `f` F18A_TEXT80X30

Examples:
```
GFXBITMAP         GRAPHICS2 only (default)
GFXBITMAP 12      GRAPHICS1 then GRAPHICS2
GFXBITMAP 12m     GRAPHICS1, GRAPHICS2, MULTICOLOR
GFXBITMAP n 12m   same without drawing
```

Used to bisect a reset seen after leaving GRAPHICS2 in `GFXMODES`. No terminal
output is produced while a graphics mode is active.
