#include <fc_api.h>

int hexdigit(int v) {
   if (v > 9) {
      return v + 'A' - 10;
   } else {
      return v + '0';
   }
}

void ansi_table() {
   fc_term_puts("  ");
   for(int c=0; c<16; c++) {
      fc_term_putc(hexdigit(c));
      fc_term_putc(' ');
   }
   fc_term_putc('\n');
   fc_term_putc(' ');
   fc_term_putc(0xd5);
   for(int c=0; c<31; c++) {
      fc_term_putc(0xcd);
   }
   fc_term_putc(0xb8);
   fc_term_putc('\n');
   for(int r=0; r<16; r++) {
      fc_term_putc(hexdigit(r));
      fc_term_putc(0xb3);
      for(int c=0; c<16; c++) {
         int value = (r << 4) + c;
         if (value < ' ') {
           fc_term_putc(27);
         }
         fc_term_putc(value);
         if (c < 15) {
           fc_term_putc(' ');
         }
      }
      fc_term_putc(0xb3);
      fc_term_putc('\n');
   }
   fc_term_putc(' ');
   fc_term_putc(0xc0);
   for(int c=0; c<31; c++) {
      fc_term_putc(0xc4);
   }
   fc_term_putc(0xd9);
   fc_term_putc('\n');
}

int fc_main(char* args) {
  ansi_table();

  return 0;
}
