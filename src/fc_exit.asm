    pseg
    even

    def fc_exit

    ref procInfoPtr

; ProcInfo offsets. TI C pointers and ints are one word.
PI_EXIT_RESUME  EQU >0006
PI_EXIT_STATUS  EQU >0008
PI_EXIT_ACTIVE  EQU >000A

; Non-local exit from an executable.
; Entry: R1 = exit status.
; This routine deliberately does not return through the API trampoline or any
; executable frames, and it does not restore R10. The executable's stack page
; is still mapped at >3000, so R10's parent value is not addressable yet.
; The resume stub (run_external_resume) maps the parent page back first, then
; restores R10/R11 and returns to runExecutable().
fc_exit:
    mov @procInfoPtr,r0
    ci r0,>FFFF
    jeq fc_exit_no_context
    mov r1,r2
    mov r0,r1
    mov @PI_EXIT_ACTIVE(r1),r0
    jeq fc_exit_no_context
    mov r2,@PI_EXIT_STATUS(r1)
    mov @PI_EXIT_RESUME(r1),r0
    mov r2,r1
    b *r0

; fc_exit is only valid while an executable is active. If called outside
; that lifetime, do not branch through the invalid 0xffff procInfoPtr.
fc_exit_no_context:
    jmp fc_exit_no_context
