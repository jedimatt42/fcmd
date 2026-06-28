	DEF	_cart

; Console ROM replacement header: CPU reset vectors at BASE_ADDR
	AORG	BASE_ADDR
	DATA	>83E0		; >0000: workspace pointer (GPLWS)
	DATA	_start		; >0002: power-on reset PC

_cart:
	CLR	@BASE_ADDR	; select bank 0
	B	@_start		; branch into C init
