    REF main
    data 0xFCFC
    data 0x8000   ; SAMS page count; bit 15 marks the extended header
    data 0x0000
    data main
    data 0x0000   ; GPU blob block - 0 means no blob
    data 0x0000   ; GPU blob length - 0 means no blob
