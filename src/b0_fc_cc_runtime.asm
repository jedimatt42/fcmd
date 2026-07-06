    .globl fc_cc_deref_handle

; fc-cc runtime helper: dereference a 32-bit heap handle
;
; Entry: R1 = address of the 2-word handle
;          handle+0: [pad:4][offset:12]
;          handle+2: [page_id:16]
;
; Exit:  R1 = resident address within 0xE000-0xFFFF
;
; Preserves: R2, R3, R12 (saved on stack, restored before return)
;
; Ledger (in fc_cc_ledger, defined in api_data.c):
;   +0: last_used   — 0 = slot 0 was most recently used, 1 = slot 1
;   +2: slot0_page  — page mapped at 0xE000
;   +4: slot1_page  — page mapped at 0xF000
;
; Register usage:
;   r1  handle pointer (in), resident address (out)
;   r2  offset from handle (word 0, upper 4 bits cleared)
;   r3  page_id from handle (word 1)
;   r12 CRU base for SAMS mapper register access

    .extern fc_cc_ledger

fc_cc_deref_handle:
    dect r10
    mov  r2, *r10           ; save r2
    dect r10
    mov  r3, *r10           ; save r3
    dect r10
    mov  r12, *r10          ; save r12

    mov *r1+, r2            ; r2 = handle word 0 (pad + offset)
    mov *r1, r3             ; r3 = handle word 1 (page_id)
    sla r2, 4               ; clear upper 4 bits (pad field)
    srl r2, 4               ; r2 = offset (12 bits)

    c r3, @fc_cc_ledger+2   ; compare with slot0_page
    jeq slot0_hit

    c r3, @fc_cc_ledger+4   ; compare with slot1_page
    jeq slot1_hit

    ; Miss — evict the slot NOT marked as last_used
    mov @fc_cc_ledger, r1   ; r1 = last_used
    dec r1                  ; r1 -= 1; sets EQ if r1 was 1
    jeq evict_slot0         ; if last_used was 1 (slot 1 MRU), evict slot 0
    jmp evict_slot1         ; else evict slot 1

evict_slot0:
    ; Map page r3 into 0xE000 (SAMS register >401C)
    mov r3, r1
    swpb r1                 ; SAMS mapper expects swapped byte order
    li  r12, >1E00
    sbo 0                   ; enable mapper register writes
    mov r1, @>401C          ; write page to 0xE000 register
    sbz 0                   ; disable mapper register writes
    mov r3, @fc_cc_ledger+2 ; slot0_page = page_id
    clr r1
    mov r1, @fc_cc_ledger   ; last_used = 0 (slot 0 is now MRU)
    ai  r2, >E000
    mov r2, r1
    jmp restore_and_return

evict_slot1:
    ; Map page r3 into 0xF000 (SAMS register >401E)
    mov r3, r1
    swpb r1
    li  r12, >1E00
    sbo 0
    mov r1, @>401E          ; write page to 0xF000 register
    sbz 0
    mov r3, @fc_cc_ledger+4 ; slot1_page = page_id
    li  r1, 1
    mov r1, @fc_cc_ledger   ; last_used = 1 (slot 1 is now MRU)
    ai  r2, >F000
    mov r2, r1
    jmp restore_and_return

slot0_hit:
    clr r1
    mov r1, @fc_cc_ledger   ; last_used = 0
    ai  r2, >E000
    mov r2, r1
    jmp restore_and_return

slot1_hit:
    li  r1, 1
    mov r1, @fc_cc_ledger   ; last_used = 1
    ai  r2, >F000
    mov r2, r1
    jmp restore_and_return

restore_and_return:
    mov  *r10+, r12         ; restore r12
    mov  *r10+, r3          ; restore r3
    mov  *r10+, r2          ; restore r2
    b    *r11
