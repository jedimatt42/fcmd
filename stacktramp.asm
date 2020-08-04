    DEF stacktramp

; tramp data structure offsets
TAR_BANK  EQU >0000
RET_BANK  EQU >0002
TAR_ADDR  EQU >0004

; stack data offsets
DATA_ADDR EQU >0000
RET_ADDR  EQU >0004

stacktramp:
    ; when called, TAR_BANK holds address of the tramp data
    ; - target bank
    ; - caller bank
    ; - target function
    mov *r10, r12                       ; load tramp data
    mov r11, @RET_ADDR(r10)             ; stash caller return address
    mov @RET_BANK(r12), @RET_BANK(r10)  ; stach caller bank
    mov @TAR_ADDR(r12), r11             ; load target address
    mov @TAR_BANK(r12), r12             ; load target bank
    clr *r12                            ; switch to target bank
    bl  *r11                            ; call target
    mov @RET_BANK(r10), r12             ; load the return bank
    clr *r12                            ; switch back to source bank
    mov @RET_ADDR(r10), r11             ; restore return address
    b   *r11                            ; return to caller
