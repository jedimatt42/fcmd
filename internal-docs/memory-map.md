# ForceCommand memory map

This document describes the current ForceCommand shell, stack, dictionary, and
external executable memory layout. Addresses are CPU addresses unless marked as
SAMS page numbers.

## CPU address space

| Range | Use |
| --- | --- |
| `0x0000–0x1FFF` | Console ROM |
| `0x2000–0x3FFF` | Lower memory expansion, 8 KB |
| `0x4000–0x5FFF` | DSR space and SAMS mapper registers |
| `0x6000–0x7FFF` | Cartridge ROM space |
| `0x8000–0x9FFF` | Cartridge/device space; speech synthesizer at `>9000` |
| `0xA000–0xFFFF` | Upper memory expansion, 24 KB |

On standard SAMS hardware, RAM can be mapped only at:

- `0x2000–0x2FFF`
- `0x3000–0x3FFF`
- `0xA000–0xF000` in 4 KB windows

`0x8000` and `0x9000` cannot be used for SAMS RAM. `0x4000–0x5FFF` is where the
SAMS mapper registers themselves live when CRU bit 0 is enabled.

## FC shell lower 8 KB

The `0x0000` console build currently uses:

| Range | Contents |
| --- | --- |
| `0x2000–0x203D` | API data table: `fc_api`, `trampdata`, `stramp`, `procInfoPtr`, GCC branch table, FC-CC pointers |
| `0x203E–0x20A5` | Remaining `.data`: CROM trampoline, VDP buffers, `heap_end`, `sams_stack_page`, display settings, etc. |
| `0x20A6–0x29B1` | `.bss`, size `0x090C` (2,316 bytes) |
| `0x29B2` | `__BSS_END` |
| `0x29B2` upward | DSR list, then dictionary, then post-dictionary heap |
| `0x2F00` | `__LOWER_LIMIT` |
| `0x2F00–0x2FFF` | Unused cushion; not runtime-checked |
| `0x3000` | `__STACK_BOTTOM` |
| `0x3000–0x3FFF` | C stack, grows down from `0x4000` |
| `0x4000` | `__STACK_TOP` |

The `0x6000` cart build differs slightly:

| Range | Contents |
| --- | --- |
| `0x2000–0x2085` | `.data`, size `0x86` |
| `0x2086–0x2991` | `.bss`, size `0x090C` |
| `0x2992` | `__BSS_END` |
| `0x2F00` | `__LOWER_LIMIT` |
| `0x3000–0x3FFF` | C stack |

Key fixed symbols in the `0x0000` build:

```text
__DATA_START      0x2000
__DATA_END        0x20A6
__BSS_START       0x20A6
__BSS_END         0x29B2
system_dict       0x20AA
dsrList           0x20AE
currentDsr        0x230C
currentPath       0x2310
heap_end          0x206A
heap_ceiling      0x244C
sams_next_page    0x26DE
sams_total_pages  0x26E0
sams_stack_page   0x206C
activeProcInfo    0x26CA
__LOWER_LIMIT     0x2F00
__STACK_BOTTOM    0x3000
__STACK_TOP       0x4000
__STATS_STACK     0x1000
```

## Dynamic lower-memory region

The area from `0x29B2` to `0x2F00` is shared dynamically:

```text
__BSS_END
  ↓
DSR list allocations
  ↓
dictionary entries
  ↓
post-dictionary heap allocations
  ↓
__LOWER_LIMIT (0x2F00)
```

### DSR list

`loadDriveDSRs()` runs before `dict_init()` and allocates one
`DeviceServiceRoutine` per discovered drive plus a terminating sentinel:

```text
16 × (number of drive DSRs + 1) bytes
```

This list starts at `__BSS_END`. For example, with 3 drive DSRs:

```text
0x29B2 + 16 × 4 = 0x29F2
```

The dictionary starts at the heap pointer after these allocations, not
necessarily at `0x29B2`.

### Dictionary

`system_dict` entries live at the post-DSR `heap_end` and grow upward:

```c
struct DictEntry {
    char type;
    char keylen;
    char vallen;
    char data[];   // key NUL value NUL
};
```

Entry size is `5 + keylen + vallen` bytes.

`dict_put()` checks the projected new end against:

- `__LOWER_LIMIT` (`0x2F00`)
- `heap_ceiling`, the lowest live allocation made after dictionary
  initialization, if any

If either would be crossed, `dict_put()` returns failure and `var_set()` or
`alias_set()` reports that the environment or alias table is full. A rejected
update leaves the previous value intact.

Pre-dictionary allocations such as the DSR list live below the dictionary and
do not constrain dictionary growth. A live post-dictionary heap allocation does
constrain it until that allocation is freed.

### Heap

`alloc()` allocates upward from `heap_end`. It:

- moves `heap_end` above the current dictionary end
- refuses allocations past `__LOWER_LIMIT`
- keeps at least 512 bytes between the heap and the current stack pointer
- records `heap_ceiling` on the first allocation made after dictionary
  initialization

`free()` restores `heap_end` and clears `heap_ceiling` when the heap is fully
released.

## C stack

`crt0.c` sets the stack pointer to `__STACK_TOP` (`0x4000`) after initializing
SAMS. Lower-expansion page 1 holds the shell's C stack:

```text
0x3000–0x3FFF   4,096-byte stack window
0x4000          initial SP
```

The shell and every executable use the full window, but with different physical
SAMS pages:

- the shell runs on SAMS page 1;
- before an executable is entered, `handleExecutable()` allocates one SAMS page
  and `run_external()` maps it at `0x3000`, then sets `r10 = 0x4000`;
- on return, `run_external_resume` maps the caller's page back before restoring
  `r10` and `r11`;
- `fc_exit` jumps to the same resume path instead of restoring `r10` itself.

Consequences:

- each executable and each nested executable gets a fresh 4 KB stack;
- the caller's stack page is preserved while a child runs;
- external API calls, banked calls, and callbacks use the executable's stack;
- `ProcInfo` cannot live on the executable stack, because that page is unmapped
  while the caller is suspended. The active context is `activeProcInfo` in
  page 0, reached through `procInfoPtr`;
- the resume path uses no stack until the caller's page is mapped back.

There is no runtime stack guard. The `0x2F00–0x2FFF` range is unused cushion;
compiled code can run past it before control returns.

External programs must not call `sams_map_page` for the `0x3000` window: it is
the running C stack, and remapping it invalidates the caller's frames.

## Upper expansion and SAMS

SAMS is required. `init_sams()` detects the installed pages, and `main()` refuses
to start unless at least `SAMS_MIN_PAGES` (32 pages, 128 KB) are present. That
is the smallest size SAMS hardware was produced in.

SAMS pages are 4 KB. At FC startup:

| SAMS page | Mapped at | Owner |
| --- | --- | --- |
| 0 | `0x2000` | FC lower `.data` / `.bss` |
| 1 | `0x3000` | Shell C stack |
| 2 | `0xB000` | Command history |
| 3 | `0xC000` | Command history |
| 4+ | `0xA000`… | External program code/data, then one private stack page |

`init_sams()` maps page 0 at `0x2000`, page 1 at `0x3000`, and sets
`sams_next_page = 2`. `history_init()` allocates pages 2 and 3, so the first
external SAMS program normally gets base page 4. The executable's private
stack page is allocated after its code/data pages, so it is the highest page
in that program's block and is freed with the rest on return.

### Paged external executable

A paged external program uses:

| Range | Contents |
| --- | --- |
| `0xA000–0xBFFF` | Currently mapped 8 KB code bank, two SAMS pages |
| `0xC000–0xFFFF` | Data/BSS window, four SAMS pages |
| `0x3000–0x3FFF` | The executable's private 4 KB C stack |

The loader reads `codePages` from the program header, then allocates:

```text
codePages + max(4, dataPages) + 1 stack page
```

Code pages alternate at `0xA000` and `0xB000`. Data pages map at `0xC000`,
`0xD000`, `0xE000`, and `0xF000`. The stack page maps at `0x3000` only while
the program runs.

Example page use, assuming base page 4:

| Program | Code pages | Data pages | Stack | Total |
| --- | --- | --- | --- | --- |
| FCARC | 4–11 (8 pages) | 12–15 (4 pages) | 16 | 13 pages |
| FCUNARC | 4–13 (10 pages) | 14–17 (4 pages) | 18 | 15 pages |

Only the first data page contains the loaded `.data` image. The external
`main()` zeroes `.bss` across the remaining data pages.

`stramp` maps a target 8 KB code bank into `0xA000`/`0xB000`, calls the target,
then remaps the caller's bank. The lower stack page is not touched by code-bank
switching.

On return, `run_external_resume` maps the caller's page back at `0x3000`, then
`handleExecutable()` frees the external program's SAMS pages (including its
stack page) and restores the previous `0xA000–0xF000` mapping snapshot.

### Flat external executable

A flat (non-paged) FC executable is loaded into six freshly allocated SAMS pages
mapped at `0xA000–0xFFFF` and gets its own 4 KB stack page at `0x3000`. On
return, `handleExecutable()` frees those pages and restores the previous
mapping.

## FC-CC paged heap

`src/fc_cc_heap.c` implements the Expanse `sams_malloc` / `sams_calloc` /
`sams_free` heap.

- `MAX_HEAP_PAGES` is 256 concurrent heap pages.
- The resident table stores only the page id plus a full-page flag.
- Each heap page stores its own free-list head in the first word of the page.
- Blocks are allocated within a single 4 KB page; handles contain a 12-bit
  offset and a 16-bit SAMS page id.

The heap is separate from the lower-expansion `alloc()` heap used by the shell.

## Inspecting the live layout

Use:

```text
sysinfo /m
```

This adds a line like:

```text
Dict:    29C4  SP: 3F1E  Stack: 1
```

`Dict:` is the current `system_dict.end`. `SP:` is the live `r10` inside
`handleSysInfo()` while printing. `Stack:` is the SAMS page currently mapped at
`0x3000` (`sams_stack_page`): 1 in the shell, or the running executable's
private stack page.
