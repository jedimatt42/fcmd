    REF main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0000   ; SAMS page count - no sams required
    data 0xFCFC   ; Flag that program promises to use screen safely
    data main     ; program start address
