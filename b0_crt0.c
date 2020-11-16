/* Entry point for C runtime initilization code
*
* This will set initial values for data memory and otherwise create
* the conditions needed for the C environment */
#include "banks.h"

#define MYBANK BANK(0)

#include "b0_sams.h"

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

  // Initialize the memory systems, map sams pages before we load
  // data into them.
  // Note: this uses c-stack, but it must be in scratchpad or all the
  // bank switching of ram will screw up the return addresses.
  __asm__("\tli sp, >8340");
  init_sams();

  /* Create the stack before declaring any variables */
  {
    extern int __STACK_TOP;
    __asm__("\tli sp, %0" : : "i" (&__STACK_TOP) );
  }

  // Fill .data section with initial values
  {
    // Filling memory with initial state, will overwrite our sams configuration data..
    // so back it up here, and then restore it later.
    int tmp_next_page = sams_next_page;
    int tmp_total_pages = sams_total_pages;

    {
      extern int __LOAD_DATA;
      extern int __DATA_START;
      extern int __DATA_END;
      char *src = (char *)&__LOAD_DATA;
      char *dst = (char *)&__DATA_START;
      while (dst < (char *)&__DATA_END)
      {
        *dst++ = *src++;
      }
    }

    /* Init .bss section to all zeros-0x0000 */
    {
      extern int __BSS_START;
      // extern int __BSS_END;
      extern int __STACK_TOP; // assuming stack is in contiquous memory with BSS
      char *dst = (char*)&__BSS_START;
      // while(dst < (char*)&__BSS_END)
      while(dst < (char*)&__STACK_TOP)
      {
        *dst++ = 0;
      }
    }

    // restore sams config
    sams_next_page = tmp_next_page;
    sams_total_pages = tmp_total_pages;
  }

  /* Start running C code */
  main();

  __asm__("blwp @>0000");
}

