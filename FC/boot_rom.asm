* boot_rom.asm - Switch to bank 0 and enter FCMD
*
* For console ROM replacement builds (BASE_ADDR=0x0000).
* After the console ROM boots, run this EA5 program to
* switch back to FCMD at _start (>0220):
*   1. Select bank 0 by clearing >0000
*   2. Branch to FCMD entry point

       AORG >2000

       LIMI 0          ; disable interrupts
       CLR  @>0000     ; select bank 0 (FCMD)
       B    @_START_ADDR     ; jump to FCMD _start

       END >2000
