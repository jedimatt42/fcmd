; Implementation of cartridge bankswitching trampoline.
; paired with BANK_CALL macro in banking.h

	DEF trampoline

; code to handle bouncing between banks.
; 
;   r11 - return address into caller function 
; tramp_data will point to a block of 3 words (in ROM):
;   [0] - return bank address
;   [1] - address of function to call
;   [2] - bank address for function to call
;
; current r11/return address and return bank need to be
; saved to the bank_stack. 
trampoline:
	; store return address onto bank_stack
	mov r11,@tramp_stash			; stash caller return address
	mov @bank_return,r11			; get pointer to bank_stack
	mov @tramp_stash,*r11			; push caller return to bank_stack
	inct @bank_return			; move to next slot in bank_stack

	; store the return bank onto stack
	mov @tramp_data,r11			; get pointer to return bank
	mov *r11,@tramp_stash			; stash return bank
	mov @bank_return,r11			; get pointer free slot in bank_stack
	mov @tramp_stash,*r11			; push caller bank to bank_stack
	inct @bank_return			; move to next slot in bank_stack
	
	; stash the target address
	inct @tramp_data			; advance to target function
	mov @tramp_data,r11			; get pointer to target function address
	mov *r11,@tramp_stash			; stash target function address

	; set destination bank
	inct @tramp_data			; advance to target bank
	mov @tramp_data,r11			; get pointer to target bank
	mov *r11,r11				; load bank address so we can write to it
	clr *r11				; to switch banks

	; call target address
	mov @tramp_stash,r11			; copy the function address so we can call it
	bl *r11					; call the target function

	; restore bank
	dect @bank_return			; move bank_stack back to caller bank
	mov @bank_return,r11			; get pointer to caller bank address
	mov *r11,r11				; load bank address so we can write to it
	clr *r11				; switch back to caller bank

	; restore return address
	dect @bank_return			; move bank_stack back to return address
	mov @bank_return,r11			; get pointer to return address
	mov *r11,r11				; load return address so we can go there

	; return to caller B *r11
	RT
	

