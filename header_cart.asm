	DEF	_cart

; Standard cartridge ROM header
	AORG	BASE_ADDR
	BYTE	>AA		; Standard Header
	BYTE	>01		; version
	BYTE	>01		; # of application programs
	BYTE	>00		; reserved
	DATA	>0000 		; power up list (unused)
	DATA	menua 		; program menu entry list
	DATA	>0000		; unused - DSR list
	DATA	>0000		; unused - subprogram list
	DATA	>0000		; unused - interrupt list
	DATA	>0000		; unused - ???

; program entry
menua:
	DATA	>0000		; next list entry
	DATA	_cart		; asm entry
	BYTE	9		; length of menu string
	TEXT	"FORCE CMD"	; menu text
	EVEN

_cart:
	CLR	@BASE_ADDR	; select bank 0
	B	@_start		; branch into C init
