     DEF _cart

; Console ROM replacement header: CPU reset vectors at BASE_ADDR
	AORG	BASE_ADDR
	DATA	>83E0		; >0000: workspace pointer (GPLWS)
	DATA	_boot		; >0002: power-on reset to console ROM

; Copy bank-switch-and-reset routine to scratchpad RAM (>8300)
; so we can execute it coherently after switching banks.
_boot:
    LWPI >8300
	LI   r1, >c800  ; mov r0, @>0000 (switch to console rom bank)
	CLR  r2
	LI   r3, >0420  ; blwp @>0000 (console reset)
	CLR  r4
	SETO r0
	B    @>8302

_cart:
    B    @_start
