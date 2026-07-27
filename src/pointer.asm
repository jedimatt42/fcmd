	def gfx_ptr_normal_black
	def gfx_ptr_normal_white
	def gfx_ptr_busy_black
	def gfx_ptr_busy_white
	def gfx_ptr_scroll_up_black
	def gfx_ptr_scroll_up_white
	def gfx_ptr_scroll_down_black
	def gfx_ptr_scroll_down_white
	def gfx_ptr_page_up_black
	def gfx_ptr_page_up_white
	def gfx_ptr_page_down_black
	def gfx_ptr_page_down_white

; 16x16 sprite pattern pairs (black outline, white fill)
; Byte order matches TI VDP format:
;   bytes 0-15: left halves of rows 0-15
;   bytes 16-31: right halves of rows 0-15
; (Same order as SPR16X macro from Virgil)

gfx_ptr_normal_black
	DATA >FF81,>8284,>8291,>A8C4
	DATA >0201,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>8040
	DATA >8000,>0000,>0000,>0000

gfx_ptr_normal_white
	DATA >007E,>7C78,>7C6E,>4703
	DATA >0100,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>0080
	DATA >0000,>0000,>0000,>0000

gfx_ptr_busy_black
	DATA >FF81,>8284,>8291,>A8C4
	DATA >0201,>7F43,>776F,>437E
	DATA >0000,>0000,>0000,>8040
	DATA >80F8,>08D8,>B808,>F800

gfx_ptr_busy_white
	DATA >007E,>7C78,>7C6E,>4703
	DATA >0100,>003C,>0810,>3C00
	DATA >0000,>0000,>0000,>0080
	DATA >0000,>F020,>40F0,>0000

gfx_ptr_scroll_up_black
	DATA >0814,>2249,>94A2,>4100
	DATA >0000,>0000,>0000,>0000
	DATA >0000,>0000,>0080,>8000
	DATA >0000,>0000,>0000,>0000

gfx_ptr_scroll_up_white
	DATA >0008,>1C36,>6341,>0000
	DATA >0000,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>0000

gfx_ptr_scroll_down_black
	DATA >41A2,>9449,>2214,>0800
	DATA >0000,>0000,>0000,>0000
	DATA >0080,>8000,>0000,>0000
	DATA >0000,>0000,>0000,>0000

gfx_ptr_scroll_down_white
	DATA >0041,>6336,>1C08,>0000
	DATA >0000,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>0000

gfx_ptr_page_up_black
	DATA >0814,>2249,>94A2,>4914
	DATA >2249,>94A2,>4100,>0000
	DATA >0000,>0000,>0080,>8000
	DATA >0000,>0080,>8000,>0000

gfx_ptr_page_up_white
	DATA >0008,>1C36,>6341,>0008
	DATA >1C36,>6341,>0000,>0000
	DATA >0000,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>0000

gfx_ptr_page_down_black
	DATA >41A2,>9449,>2214,>49A2
	DATA >9449,>2214,>0800,>0000
	DATA >0080,>8000,>0000,>0080
	DATA >8000,>0000,>0000,>0000

gfx_ptr_page_down_white
	DATA >0041,>6336,>1C08,>0041
	DATA >6336,>1C08,>0000,>0000
	DATA >0000,>0000,>0000,>0000
	DATA >0000,>0000,>0000,>0000
