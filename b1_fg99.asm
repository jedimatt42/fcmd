       def fg99

; Entry point copied to RAM
fgstart:
       li   r2, 20            ; sequence length: prefix (8) + sender (12)
fgsend:
       clr  @>6000            ; signal new byte
       li   r1, >0038         ; >7000 >> 9
       movb *r0+, r1
       src  r1, 7             ; >7000 + (byte << 1)
       clr  *r1               ; send byte

       dec  r2
       jne  fgsend

       clr  @>6000            ; done

fgwait:
       ; wait for image to be loaded
       src  r0, 8             ; burn at least 21 cycles
       src  r0, 8             ; burn at least 21 cycles
       src  r0, 8             ; burn at least 21 cycles
       src  r0, 8             ; burn at least 21 cycles
       li   r0, >6000         ; check >6000->6200
       li   r2, >100
fgl1   mov  *r0+, r1
       jne  fgdone
       dec  r2
       jne  fgl1
       jmp  fgwait

       ; image has been loaded
fgdone:
       ; blwp @>0000  -- reset console
       lwpi >83E0
       mov  r6,r6             ; test for 0x0000
       jeq  fgrst
       li   r7, >60           ; if not zero, re-enter GPL and continue to that GROM address
       b    *r7
fgrst:
       blwp @>0000
endfg:

; Entry point in ROM
fg99:
       mov   r1, r0           ; take message address from gcc caller
       mov   r2, r6           ; take grom launch address from gcc caller (0x0000 causes console reset)

; the next steps will take the ROM code out from under the CPU, so we need to copy it up to
; expansion RAM and continue from there. All the code below must be relocatable.
       li   r1,fgstart        ; source address to copy from
       li   r3,endfg          ; length of code to copy
       s    r1,r3             ;   is now in r3
       li   r2,>A000          ; target address in ram to copy into
fgcopy:
       mov  *r1+,*r2+         ; copy a word, increment src and dest
       dect r3                ; count down by 2 bytes
       jne  fgcopy            ; continue if r3 is not zero
       b    @>A000            ; run code from ram