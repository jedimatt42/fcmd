       def fg99
       def fg99_msg
       def fg99_addr

fg99:
       li   r0, fgmenu_seq    ; this is the only non-relocatable instruction
       li   r2, 20            ; sequence length: prefix (8) + sender (12)
fgsend:
       clr  @>6000            ; signal new byte
       li   r1, >0038         ; >7000 >> 9
       movb *r0+, r1
       src  r1, 7             ; >7000 + (byte << 1)
       clr  *r1               ; send byte
;      mov  *r1, r3           ; in RAM mode, use these lines instead
;      mov  r3, *r1           ; to send a byte

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
       mov  @fg99_addr,r6
       jeq  fgrst
       li   r7, >60
       b    *r7
fgrst:
       blwp @>0000

fgmenu_seq:
       ; send this to reload
       byte >99
       text 'OKFG99'
       byte >99
fg99_msg:
       data >0000, >0000, >0000, >0000    ; file to load (8 chars, pad with \00)
gr_flag:
       data >0000                         ; >0000 for GROM/mixed, >FFFF for ROM
fg99_addr:
       data >0000
