    REF fc_main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0004   ; SAMS page count - 0x0004 means sams required, 2 banks of 2 pages - 4
    data 0xFCFC   ; Flag that program promises to use screen safely
    data fc_main     ; program start address
