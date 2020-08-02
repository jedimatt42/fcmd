    DEF stacktramp

RET_ADDR  EQU >0004
RET_BANK  EQU >0002
TAR_BANK  EQU >0000

stacktramp:
    mov @RET_ADDR(r10), r0      ; load target function address
    mov r11, @RET_ADDR(r10)     ; store return address back on stack
    mov @TAR_BANK(r10),r12      ; load the target bank
    clr *r12                    ; switch to target bank
    bl  *r0                     ; run target function
    mov @RET_BANK(r10),r12      ; load the return bank
    clr *r12                    ; switch back to source BANK_STACK_SIZE
    mov @RET_ADDR(r10), r11     ; restore return address
    b   *r11                    ; return to caller
