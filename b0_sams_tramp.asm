    DEF stramp
    REF trampdata

; tramp data structure offsets
TAR_BANK  EQU >0000
RET_BANK  EQU >0002
TAR_ADDR  EQU >0004
STASH_R13 EQU >0006

; stack data offsets
DATA_ADDR EQU >0004
RET_ADDR  EQU >0000

; bottom bank addr - pre shifted for SAMS
BOT_ADDR  EQU >4014
; top bank addr - pre shifted for SAMS
TOP_ADDR  EQU >4016


stramp:
    ; when called, trampdata holds address of the tramp data
    ; - target bank
    ; - caller bank
    ; - target function
    ; caller cheats and didn't adjust stack
    mov @trampdata,r13
    ai  r10, -8                         ; consume stack space
    mov r11, *r10                       ; stash caller return address
    mov @RET_BANK(r13), @RET_BANK(r10)  ; stash caller bank
    mov r13, @STASH_R13(r10)            ; stash r13 so we can use it.
    mov @TAR_ADDR(r13), r11             ; load target address
    mov *r13, r13                       ; load target bank

    ; switch to target bank
    li  r12, >1E00                      ; sams mapper crubase
    swpb r13
    sbo 0
    mov r13, @BOT_ADDR                  ; map page into >A000
    inct r13
    mov r13, @TOP_ADDR                  ; map page+1 into >B000
    sbz 0

    bl  *r11                            ; call target
    mov @RET_BANK(r10), r13             ; load the return bank

    ; switch back to source bank
    li  r12, >1E00                      ; sams mapper crubase
    swpb r13
    sbo 0
    mov r13, @BOT_ADDR                  ; original page into >A000
    inct r13
    mov r13, @TOP_ADDR                  ; map original page+1 into >B000
    sbz 0

    mov @RET_ADDR(r10), r11             ; restore return address
    mov @STASH_R13(r10), r13            ; restore r13
    ai  r10, 8                          ; restore stack location
    b   *r11                            ; return to caller

