    REF main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x8006   ; SAMS page count; bit 15 marks the extended header
    data 0x0000   ; Flag that program requires screen reset on exit
    data main   ; program start address
    data 0x0000   ; GPU blob block - 0 means no blob
    data 0x0000   ; GPU blob length - 0 means no blob
