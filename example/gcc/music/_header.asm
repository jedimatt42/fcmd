    REF fc_main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0000   ; SAMS page count - 0x0000 means simply / no sams required
    data 0xFCFC   ; Flag that program promises to use screen safely
    data fc_main     ; program start address
