    REF main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0006   ; SAMS page count - 0x0000 means simply / no sams required
    data 0x0000   ; Flag that program requires screen reset on exit
    data main   ; program start address
