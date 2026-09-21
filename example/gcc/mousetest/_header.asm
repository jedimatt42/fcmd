    REF main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0000   ; SAMS page count - no sams required
    data 0x0000   ; Uses the screen and sprites; let ForceCommand restore it
    data main     ; program start address
