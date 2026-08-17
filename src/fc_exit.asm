    pseg
    even

    def fc_exit_prepare
    def fc_exit

    ref procInfoPtr

; ProcInfo offsets. TI C pointers and ints are one word.
PI_EXIT_SP      EQU >0004
PI_EXIT_RESUME  EQU >0006
PI_EXIT_STATUS  EQU >0008
PI_EXIT_ACTIVE  EQU >000A

; Save the runExecutable() stack and resume address before entering the
; executable. Entry: R1 = resume address, R10 = runExecutable() stack.
fc_exit_prepare:
    ai r10,>FFFE
    mov r1,*r10
    mov @procInfoPtr,r1
    mov r10,r0
    ai r0,>0002
    mov r0,@PI_EXIT_SP(r1)
    mov *r10+,r0
    mov r0,@PI_EXIT_RESUME(r1)
    clr r0
    mov r0,@PI_EXIT_STATUS(r1)
    inc r0
    mov r0,@PI_EXIT_ACTIVE(r1)
    b *r11

; Non-local exit from an executable.
; Entry: R1 = exit status.
; This routine deliberately does not return through the API trampoline or
; any executable frames. The restored runExecutable() frame performs the
; normal ForceCommand cleanup on return.
fc_exit:
    mov @procInfoPtr,r0
    ci r0,>FFFF
    jeq fc_exit_no_context
    mov r1,r2
    mov r0,r1
    mov @PI_EXIT_ACTIVE(r1),r0
    jeq fc_exit_no_context
    mov r2,@PI_EXIT_STATUS(r1)
    mov @PI_EXIT_SP(r1),r10
    mov @PI_EXIT_RESUME(r1),r0
    mov r2,r1
    b *r0

; fc_exit is only valid while an executable is active. If called outside
; that lifetime, do not branch through the invalid 0xffff procInfoPtr.
fc_exit_no_context:
    jmp fc_exit_no_context
