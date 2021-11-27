define(BANKSECTION, .bank$1 { objects/b$1_*.o(.text) } )
define(BANKSUMMARY, __STATS_BANK_$1 = 0x2000 - 0x0080 - SIZEOF(.bank$1); )
define(`BANKSECTIONS', `BANKSECTION($1) ifelse(eval($1 > $2), 1, `
    BANKSECTIONS(decr($1), $2)', `')')dnl
define(`BANKSUMMARIES', `BANKSUMMARY($1) ifelse(eval($1 > $2), 1, `
  BANKSUMMARIES(decr($1), $2)', `')')dnl
define(BANKROMSIZE, SIZEOF(.bank$1) )
define(`TOTALROMSPACE', `BANKROMSIZE($1) + ifelse(eval($1 > $2), 1, `TOTALROMSPACE(decr($1), $2)', `')')dnl

define(TOP_BANK, 15)

MEMORY
{
  cart_rom   (rx) : ORIGIN = 0x6000, LENGTH = 0x0080 /* 128 bytes up front of common code */
  bank_rom   (rx) : ORIGIN = 0x6080, LENGTH = 0x2000 - 0x0080 /* 8k-header of bankable cartridge ROM */
  lower_exp  (wx) : ORIGIN = 0x2000, LENGTH = 0x2000 /* 8k        */
  upper_exp  (wx) : ORIGIN = 0xA000, LENGTH = 0x6000 /* 24k upper ram */
  scratchpad (wx) : ORIGIN = 0x8320, LENGTH = 0x00e0 /* 32b is for workspace */
}

SECTIONS
{
  /* common */
  .text : {              /* all modules listed here will end up at front of each ROM bank */
    objects/header.o(.text)
    objects/trampoline.o(.text)
    objects/trampdata.o(.text)
    objects/ea5_reset.o(.text)
    objects/str_memcpy.o(.text)  /* gcc will implicity call this to initialize strings on stack */
  } >cart_rom

  /* overlays */
  OVERLAY : AT (0x10000)
  {
    .bank0 {             /* list the set of modules in bank 0 */
      objects/api.o(.text)
      objects/b0_*.o(.text)
      __LOAD_DATA = .;   /* .data segment is appended here by makefile for final ROM .bin */
    }
    BANKSECTIONS(TOP_BANK,1)
  } >bank_rom

  .data : {
    __DATA_START = .;    /* define RAM location symbol so __LOAD_DATA can be copied from ROM */
    objects/trampdata.o (.data);
    * (.data);
    __DATA_END = .;      /* identifies end of data for initialization routine. */
  } >lower_exp

  /* We overflow off the end of .bss on purpose so this must go after .data */
  .bss  ALIGN(2) : {
    __BSS_START = .;     /* b0_crt0 will initialize RAM in here to 0x00 values */
    *(.bss);
    __BSS_END = .;
  } >lower_exp

  __STACK_TOP = 0x4000;

  __STATS_DATA_BSS = SIZEOF(.data) + SIZEOF(.bss);
  __STATS_STACK = __STACK_TOP - __BSS_END;

  __STATS_HEAD = SIZEOF(.text);
  BANKSUMMARIES(TOP_BANK,0)
  __TOTAL_ROM_USED = TOTALROMSPACE(TOP_BANK,0) 0;
}
