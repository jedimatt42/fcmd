    DEF trampoline
    REF trampdata

; tramp data structure offsets
TAR_BANK  EQU >0000
RET_BANK  EQU >0002
TAR_ADDR  EQU >0004

; stack data offsets
RET_ADDR  EQU >0000
; RET_BANK    >0002
DATA_ADDR EQU >0004
R12_STASH EQU >0006

trampoline:
    ; when called, trampdata holds address of the tramp data
    ; - target bank
    ; - caller bank
    ; - target function
    ; caller cheats and didn't adjust stack
    ai  r10, -8                         ; consume stack space
    mov r11, *r10                       ; stash caller return address
    mov r12, @R12_STASH(r10)            ; stash r12 in case it was in use...
    mov @trampdata,r12                  ; put trampdata address in index register
    mov @RET_BANK(r12), @RET_BANK(r10)  ; stash caller bank
    mov @TAR_ADDR(r12), r11             ; load target address
    mov *r12, r12                       ; load target bank
    clr *r12                            ; switch to target bank
    bl  *r11                            ; call target
    mov @RET_BANK(r10), r12             ; load the return bank
    clr *r12                            ; switch back to source bank
    mov @RET_ADDR(r10), r11             ; restore return address
    mov @R12_STASH(r10), r12            ; restore r12, since we stepped on it
    ai  r10, 8                          ; restore stack location
    b   *r11                            ; return to caller

