    pseg
    even

    def run_external
    def run_external_resume

    ref procInfoPtr
    ref sams_stack_page

; ProcInfo offsets. These must match src/procinfo.h.
PI_BASE_PAGE   EQU >0000
PI_EXIT_SP     EQU >0004
PI_EXIT_RESUME EQU >0006
PI_EXIT_STATUS EQU >0008
PI_EXIT_ACTIVE EQU >000A
PI_EXIT_PAGE   EQU >000C
PI_RET_ADDR    EQU >000E
PI_STACK_PAGE  EQU >0010
PI_SAVED_R12   EQU >0012

; int run_external(int stack_page, int main_addr, int arg)
;
; Enter an executable on its own SAMS stack page at >3000. This routine runs
; from ROM and uses no stack until it has switched to the new page, so it is
; safe to remap the page that held the caller's C frame.
;
; On entry:
;   r1 = new stack page
;   r2 = executable main address (from >A006)
;   r3 = argument string
;   r10 = parent stack pointer
;   r11 = return address into runExecutable()
;
; The parent stack pointer, parent >3000 page, and the C return address are
; kept in the resident ProcInfo so fc_exit can use the same resume path.
run_external:
    mov  @procInfoPtr,r6
    mov  r10,@PI_EXIT_SP(r6)
    mov  r11,@PI_RET_ADDR(r6)
    mov  @sams_stack_page,r4
    mov  r4,@PI_EXIT_PAGE(r6)
    mov  r1,@PI_STACK_PAGE(r6)
    mov  r12,@PI_SAVED_R12(r6)

    li   r5,run_external_resume
    mov  r5,@PI_EXIT_RESUME(r6)
    li   r5,>0001
    mov  r5,@PI_EXIT_ACTIVE(r6)

    ; Map the executable's stack page at >3000. No stack accesses after this
    ; point until r10 has been moved into the new page.
    li   r12,>1E00
    swpb r1
    sbo  0
    mov  r1,@>4006
    sbz  0
    swpb r1
    mov  r1,@sams_stack_page

    mov  @PI_SAVED_R12(r6),r12
    li   r10,>4000
    mov  r3,r1
    bl   *r2

; Common resume path for a normal main() return and for fc_exit().
; Entry: r1 = status. The executable's page is still at >3000, so do not use
; the stack until the parent page has been restored.
run_external_resume:
    mov  r1,r3
    mov  @procInfoPtr,r6
    mov  @PI_EXIT_PAGE(r6),r1

    li   r12,>1E00
    swpb r1
    sbo  0
    mov  r1,@>4006
    sbz  0
    swpb r1
    mov  r1,@sams_stack_page

    mov  @PI_SAVED_R12(r6),r12
    mov  @PI_EXIT_SP(r6),r10
    mov  @PI_RET_ADDR(r6),r11
    clr  r5
    mov  r5,@PI_EXIT_ACTIVE(r6)
    mov  r3,r1
    b    *r11
