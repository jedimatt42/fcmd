    REF main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0000   ; SAMS page count - no sams required
    data 0x0000   ; Not screen-safe: ForceCommand restores text on return
    data main     ; program start address
