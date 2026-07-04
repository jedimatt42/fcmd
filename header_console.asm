	DEF _cart
	DEF _boot
	DEF _vdp_isr

; Console ROM replacement header: CPU reset vectors at BASE_ADDR
	AORG	BASE_ADDR
	DATA	>83E0		; >0000: workspace pointer (GPLWS)
	DATA	_boot		; >0002: power-on reset to console ROM
	DATA    >83C0       ; >0004: interrupt 1 workspace
	DATA    _vdp_isr    ; >0006: should be an empty isr, but I have space issues.
	DATA    >83C0       ; >0008: interrupt 2 workspace
	DATA    _vdp_isr    ; >000A: empty interrupt routine
; Should define interrupt 3, but isn't reachable on TI-99/4A
;	DATA    >83C0       ; >000C: interrupt 3 workspace
;	DATA    _nopint     ; >000E: empty interrupt routine

; Copy bank-switch-and-reset routine to scratchpad RAM (>8300)
; so we can execute it coherently after switching banks.
_boot:
	LWPI >8300      ; we'll use register loads to put machine code for bank switch and reset into ram.
	LI   r1, >0720  ; seto @>0000 (switch to console rom bank)
	CLR  r2
	LI   r3, >0420  ; blwp @>0000 (console reset)
	CLR  r4
	B    @>8302

_cart:
	B    @_start

_vdp_isr:
	MOVB @>8802, R0
	RTWP
