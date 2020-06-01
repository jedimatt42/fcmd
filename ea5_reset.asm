    DEF GOEA5

; Assumes the launch address is in GPLWS R11
; This provides a safe location to return to from an ea5 program
; as it will be in every bank of the ROM at the same address
GOEA5:
	LWPI	>83E0		; some programs save R11 from GPLSWS instead of current WS.
	BL	    *R11	    ; enter the program
	BLWP	@>0000		; if it returns here, then reset