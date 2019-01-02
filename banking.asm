; Implementation of cartridge bankswitching trampoline.
; paired with FAR_CALL macro in banking.h

	DEF tramp_func
	DEF trampoline

; allows casting to target function signature more easily.
;  although I know it wastes a word.
tramp_func:
	DATA trampoline

RTADDR	EQU	>8300+(11*2)		; c register r11

; dest_bank, dest_func_addr, bank_return will have been set already.
;   bank_return will be the bank value in the stack. 
;   we still need to capture and set the return address and save
;   it to the stack.
trampoline:
	; advance bank_return stack pointer
	INCT @bank_return

	; need to capture return addr
	MOV @RTADDR,@tramp_stash	; stash the return addr so I can use r11
	MOV @bank_return,r11
	MOV @tramp_stash,*r11
	; move pointer forward ready for nested BANK_CALL from C.
	INCT @bank_return

	; switch bank
	MOV @dest_bank,r11
	CLR *r11

	; call the intended function
	; all the arguments should be in their places on the stack
	; still or in registers as per calling convention.
	MOV @dest_func_addr,r11
	BL *r11

	; restore bank
	DECT @bank_return		; rewind to the return address
	MOV @bank_return,r11		; 
	MOV *r11,@tramp_stash		; stash the original return address
	DECT @bank_return		; rewind to the originating bank
	MOV @bank_return,r11		; 
	MOV *r11,r11			; 
	CLR *r11			; switch to the return bank

	; restore caller return addr
	MOV @tramp_stash,r11		; grab the return address
	RT				; return to caller B *r11
	

