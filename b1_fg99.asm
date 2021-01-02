       def fg99

GPLWSR6       EQU    >83EC

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
       jne  fgdelay
       dec  r2
       jne  fgl1
       jmp  fgwait

       ; ROM image has been loaded
       li   r2, >1000
fgdelay:
       ; wait for GROM to have a chance to be loaded
       src  r0, 8             ; burn at least 21 cycles
       src  r0, 8             ; burn at least 21 cycles
       src  r0, 8             ; burn at least 21 cycles
       src  r0, 8             ; burn at least 21 cycles
       dec  r2
       jne  fgdelay

fgdone:
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
; Move passed in message data to scratchpad
       li    r3, >8320        ; set destination
       ; r1 is already source
       li    r4, 10
fgcp:
       mov   *r1+,*r3+        ; copy word
       dec   r4
       jne   fgcp
       li    r1, >8320        ; set new address of message

; In case of SAMS, re-align page mapping
       li    r12, >1e00       ; set SAMS crubase
       sbo   0                ; make mapping registers accessible
       li    r4, 0            ; page 0
       li    r6, >0000        ; -> address >0000
       li    r5, 16
fgmp:
       mov   r6, r3
       srl   r3, 12
       sla   r3, 1
       mov   r4, @>4000(r3)   ; map page
       ai    r4, >0100        ; next page
       ai    r6, >1000        ; next address
       dec   r5               ; count pages
       jne   fgmp

       sbz   0                ; hide registers
       sbz   1                ; turn mapper off
; proceed to bound cartridge
       mov   r1, r0           ; take message address from gcc caller
       mov   r2, @GPLWSR6     ; take grom launch address from gcc caller (0x0000 causes console reset)
       mov   r2, @18(r1)      ; fill launch address into old structure ( voodoo )

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
