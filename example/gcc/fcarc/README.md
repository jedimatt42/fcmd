# FCARC — Force Command Archive

`FCARC` (create) and `FCUNARC` (list/extract/test) are external Force Command
executables. They support:

- a new FCARC v1 format with internal directories
- the legacy TI Archiver (ARC/ARK) format, compressed and uncompressed
- per-file LZW compression (the same LZW bitstream used by TI Archiver)
- exact preservation of TI file metadata (PROGRAM, DIS/INT, FIX/VAR, record
  length, EOF offset, L3 record count)
- optional host-side creation/extraction with `scripts/fcarc.py`

The legacy TI Archiver format is **LZW**, not LZH. It uses variable-width
9–12-bit codes, a clear code (256), and an end code (257).

## Requirements

- SAMS memory. Both are paged SAMS executables: `FCARC` uses 4 code banks
  (8 code pages + 4 data pages), `FCUNARC` uses 5 code banks (10 code pages
  + 4 data pages). Allow at least 14 free SAMS pages / 56 KB.
- TIPI, SCSI, IDE/HFDC, or a disk controller that supports Level 2 direct
  file I/O for the raw-sector paths.
- A Force Command build that includes `FC/BIN/FCARC` and `FC/BIN/FCUNARC`.

## Usage

```text
FCARC c <archive> <file>...              create a flat FCARC archive
FCARC c /r <archive> <dir>...            create a FCARC archive with directories
FCARC c /s <archive> <source>...         store without compression
FCARC c /ti <archive> <file>...          create a legacy TI Archiver archive
FCARC c /ti /u <archive> <file>...       create an uncompressed legacy archive

FCUNARC l <archive>                      list
FCUNARC x <archive> [<path>...]          extract all or selected paths
FCUNARC t <archive>                      test integrity
```

Examples:

```text
FCARC c /r TIPI.MYARCH TIPI.PROJECT
FCUNARC l TIPI.MYARCH
FCUNARC x TIPI.MYARCH
FCUNARC x TIPI.MYARCH PROJECT.DOCS
FCARC c /ti TIPI.OLDARC TIPI.FILE1 TIPI.FILE2
```

- `c` without `/r` accepts files only.
- `c /ti` always rejects directory sources; the legacy format is flat.
- `x` extracts into the current Force Command directory. Archive paths are
  dot-separated, e.g. `PROJECT.DOCS.README`.
- Extracted files are written as TIFILES FIAD files, so a native text file
  that the DSR presented as `DIS/FIX 128` is extracted in that form.
- `t` prints one line per file, prefixed with `OK    ` or `ERROR `, and exits
  with the number of bad file entries.
- `/s` stores file sectors without compression.
- `/q` suppresses per-file progress.

## Limits

- 120 entries per FCARC archive on the 4A.
- 127 entries per legacy archive (format limit).
- 39-character archive paths, 10 characters per component.
- 4 GB fields in the format; practical limits are imposed by the DSR.
- 32 entries per source directory are catalogued at a time.

## FCARC v1 format

All multi-byte values are little-endian.

```text
offset 0        128-byte header
offset 128      data area
...             central directory
...             128-byte trailer at the end
```

The payload is padded to a multiple of 256 bytes. The archive file itself is
stored as TI `INT/FIX 128`.

### Header

```text
0x00   8   "FCARC1\0\0"
0x08   2   version = 1
0x0A   2   flags (bit 0 = directories, bit 1 = LZW)
0x0C   32  archive comment, space/NUL padded
0x2C   84  reserved
```

### Trailer

```text
0x00   8   "FCARCEOF"
0x08   2   version = 1
0x0A   2   flags
0x0C   4   directory offset
0x10   4   directory size
0x14   2   entry count
0x16   2   reserved
0x18   4   payload size (must equal file size)
0x1C   2   directory CRC-16/CCITT
0x1E   98  reserved
```

### Directory entry

```text
0x00   1   path length
0x01   1   kind: 0 = file, 1 = directory
0x02   1   method: 0 = stored, 1 = LZW
0x03   1   TI flags
0x04   1   records per sector
0x05   1   EOF offset
0x06   1   record length
0x07   1   total data sectors, high byte
0x08   1   total data sectors, low byte
0x09   1   L3 low byte
0x0A   1   L3 high byte
0x0B   1   reserved
0x0C   4   original size (sector padded)
0x10   4   stored size
0x14   4   data offset
0x18   2   payload CRC-16/CCITT
0x1A   N   dot-separated path
```

File data is aligned to a 128-byte record boundary. Directory entries are
written after all file data. Directory entries with `kind = 1` preserve empty
directories.

## Legacy format compatibility

`FCARC l`, `x`, and `t` auto-detect the legacy format. Compressed archives are
identified by the initial `0x80` clear-code byte; uncompressed archives are
identified by the `END!` marker in the directory.

`FCARC c /ti` writes a true legacy archive:

- compressed: `INT/FIX 128`, whole-stream LZW
- uncompressed: `DIS/FIX 128`, write-protected
- 18-byte directory entries, 14 per sector, `END!` at offset `0xFC`
- raw 256-byte file sectors after the directory

The legacy format has no directory support. Use FCARC for archives with
internal directories.

## Host tool

`scripts/fcarc.py` is a dependency-free Python 3 implementation of both
formats. It reads/writes TIFILES FIAD headers and is useful for testing and
for creating archives on a modern computer.

```text
python3 scripts/fcarc.py list archive.fca
python3 scripts/fcarc.py extract archive.fca --out outdir
python3 scripts/fcarc.py create archive.fca srcdir --recursive --tifiles
python3 scripts/fcarc.py create-ti legacy.ark file1 file2 --tifiles
python3 scripts/fcarc.py test archive.fca
```

When copying a host-created archive to TIPI or an emulator, use `--tifiles` so
the file is wrapped as a proper TI `INT/FIX 128` or `DIS/FIX 128` file. On the
4A, `FCARC` creates the correct TI file type directly through the DSR.

TI permits printable characters such as `/` in file names. The 4A tool
preserves them exactly. A host filesystem cannot represent `/` in a file name,
so host extraction percent-escapes host-illegal characters: a TI file
`A/B` is extracted as `A%2FB`. The archive itself always stores `A/B`.

Run the tests with:

```text
python3 scripts/test_fcarc.py
```

## Building

```bash
make -C example/gcc/fcarc
```

The build produces `FCARC` and `FCUNARC` and copies both to `FC/BIN/`.
