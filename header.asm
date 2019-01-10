	AORG 	>6000

; Cartridge ROM header
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
	BYTE	7		; length of menu string
	TEXT	"TIPICMD"	; menu text
	EVEN

; start here instead of C typical entry of _start, so we can
; force over to bank0, as most cartridges don't guarantee
; power-on bank selection.
_cart:
	CLR	@>6000		; move to bank0
	B	@_start		; now branch into C.

	

