/* Entry point for C runtime initilization code
*
* This will set initial values for data memory and otherwise create
* the conditions needed for the C environment */

extern int main(void);

#ifdef __cplusplus
extern "C"
#endif
void _start(void)
{

  /* Disable interrupts
  *
  * This will disable console reset via keyboard, music playback,
  * sprite automotion, etc. But will make the environment much simpler
  * to work in. Users can re-enable interrupts later if they wish. */
  __asm__("\tlimi 0");

  /* Set initial workspace
  *
  * The registers will be located at the start of scratchpad memory */
  __asm__("\tlwpi >8300");

  /* Create the stack before declaring any variables */
  {
    extern int __STACK_TOP;
    __asm__("\tli sp, %0" : : "i" (&__STACK_TOP) );
  }

  // Fill .data section with initial values
  {
    extern int __LOAD_DATA;
    extern int __DATA_START;
    extern int __DATA_END;
    char *src = (char*)&__LOAD_DATA;
    char *dst = (char*)&__DATA_START;
    while(dst < (char*)&__DATA_END)
    {
      *dst++ = *src++;
    }
  } 

  /* Init .bss section to all zeros-0x0000 */
  {
    extern int __BSS_START;
    extern int __BSS_END;
    char *dst = (char*)&__BSS_START;
    while(dst < (char*)&__BSS_END)
    {
      *dst++ = 0;
    }
  }

  /* Start running C code */
  main();

  __asm__("blwp @>0000");
}

