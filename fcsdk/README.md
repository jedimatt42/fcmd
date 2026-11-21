# ForceCommand SDK for C

- fc_api.h : data structures and function declarations for ForceCommand API
- ioports.h : constants and inline utilities for VDP memory mapped io port access 
- kscan.h : constants for mode-5 kscan codes that are not in the ASCII printable character ranges
- vocab.h : constants for all the TI Speech PHROM vocabulary words
- fc.ld : Linker script defining fixed symbols for the gcc runtime in the cartridge ROM space

## Executable header

A ForceCommand executable starts with a header at `>A000`. New executables use
the extended header, including programs that have no GPU blob:

```asm
    data >FCFC
    data >8000 | sams      ; bit 15 marks the extended header
    data safe
    data main
    data gpu_blob_block    ; file block containing the GPU blob
    data gpu_blob_length   ; blob length in bytes
```

The extended header is 12 bytes, so `main` and the code begin at `>A00C`.
Zero `gpu_blob_block` or `gpu_blob_length` means there is no GPU blob.

The original 4-word header is still accepted for older executables:

```asm
    data >FCFC      ; valid ForceCommand executable
    data sams       ; SAMS page count, 0 for a flat 24K image
    data safe       ; >FCFC if the program restores the display itself
    data main       ; entry point
```

With the original header, bit 15 of `sams` is clear and code begins at
`>A008`.
`gpu_blob_block` is the program payload block (256-byte sector), not a TIFILES
header block. Append the blob on a 256-byte boundary; the loader copies it into
GRAM and restores the previous GPU allocation when the program returns. See
`internal-docs/f18a-gpu.md` for the GPU calling convention.
