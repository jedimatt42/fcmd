* ForceCommand GPU base image.
*
* This is data, not host code. It is copied into the F18A GPU GRAM at >4000.
* Layout:
*   >4000  IDLE
*   >4002  >FC01 magic
*   >4004  reserved
*   >4006  host BLKCPY entry
*   >4014  GPU-callable BLKCPY entry
*   >4018  BLKCPY core
*   >403C  first free address
*
* The same file also carries the tiny bootstrap that copies the base from
* VRAM >3000 into GRAM >4000 and initializes the GPU stack pointer.

    DEF gpu_base
    DEF gpu_boot

gpu_base:
    DATA >0340,>FC01,>0000,>06A0,>4018,>0201,>FF00,>D801
    DATA >3F0E,>0340,>0460,>4018,>C020,>3F02,>C060,>3F04
    DATA >C0A0,>3F06,>D0E0,>3F01,>1305,>D010,>DC40,>0602
    DATA >16FD,>1003,>DC70,>0602,>16FD,>045B

gpu_boot:
    DATA >0200,>3000,>0201,>4000,>0202,>001E,>CC70,>0602
    DATA >16FD,>020F,>47FE,>0200,>FF00,>D800,>3F0E,>0340
