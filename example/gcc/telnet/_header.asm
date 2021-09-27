    REF fc_main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0000   ; SAMS page count - 0x0000 means simply / no sams required
    data 0x0000   ; Flag 0xFCFC == program safely uses screen, 0x0000 == FC restore screen on return
    data fc_main     ; program start address
