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
	mov @bank_return,r12		; get pointer to bank_stack
	mov R11,*r12+			; store return address onto bank_stack
					;   and move bank_stack pointer forward

	; store the return bank onto stack
	mov @tramp_data,r0		; get pointer to trampoline data for call
	mov *r0+,*r12+			; store return bank onto bank_stack
					;   and move both pointers forward

	; set bank_return pointer to next slot in bank_stack
	mov r12,@bank_return		; r12 currently points at next free slot
	
	; stash the target address
	mov *r0+,r11			; get address of function, and move pointer
					;   forward to target bank

	; set destination bank
	mov *r0,r12			; load target bank address
	clr *r12			; switch to target bank

	; call target address
	bl *r11				; run the target function

	; restore bank
	mov @bank_return,r0		; get bank_stack pointer
	dect r0				; rewind to calling bank
	mov *r0,r11			; load return bank address
	clr *r11			; switch to calling bank

	; restore return address
	dect r0				; rewind to caller return address
	mov *r0,r11			; restore our return address

	; make sure bank_return is adjusted back
	mov r0,@bank_return		; free up this slot in bank_stack

	; return to caller B *r11
	RT
	

