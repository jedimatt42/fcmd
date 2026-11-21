    REF main
    data 0xFCFC   ; valid Force Command binary
    data 0x8008   ; SAMS page count; bit 15 marks the extended header
    data 0xFCFC   ; screen is left in a safe state
    data main
    data 0x0000   ; GPU blob block - 0 means no blob
    data 0x0000   ; GPU blob length - 0 means no blob
