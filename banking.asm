; Implementation of cartridge bankswitching trampoline.
; paired with FAR_CALL macro in banking.h

	DEF dest_bank
	DEF dest_func_addr
	DEF caller_bank
	DEF caller_return_addr
	DEF tramp_func

dest_bank:
	BSS 2
dest_func_addr:
	BSS 2
caller_bank:
	BSS 2
caller_return_addr:
	BSS 2

tramp_func:
	DATA trampoline


; dest_bank, dest_func_addr, caller_bank will have been set already.
trampoline:
	; need to capture return addr
	MOV R11,@caller_return_addr

	; switch bank
	MOV @dest_bank,R11
	CLR *R11

	; call the intended function
	; all the arguments should be in their places on the stack
	; still or in registers as per calling convention.
	MOV @dest_func_addr,R11
	BL *R11

	; restore bank
	MOV @caller_bank,R11
	CLR *R11

	; restore caller return addr
	MOV @caller_return_addr,R11
	RT
	

