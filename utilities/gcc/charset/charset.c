#include <fc_api.h>

int hexdigit(int v) {
   if (v > 9) {
      return v + 'A' - 10;
   } else {
      return v + '0';
   }
}

void ansi_table() {
   term_puts("  ");
   for(int c=0; c<16; c++) {
      term_putc(hexdigit(c));
      term_putc(' ');
   }
   term_putc('\n');
   term_putc(' ');
   term_putc(0xd5);
   for(int c=0; c<31; c++) {
      term_putc(0xcd);
   }
   term_putc(0xb8);
   term_putc('\n');
   for(int r=0; r<16; r++) {
      term_putc(hexdigit(r));
      term_putc(0xb3);
      for(int c=0; c<16; c++) {
         int value = (r << 4) + c;
         if (value < ' ') {
           term_putc(27);
         }
         term_putc(value);
         if (c < 15) {
           term_putc(' ');
         }
      }
      term_putc(0xb3);
      term_putc('\n');
   }
   term_putc(' ');
   term_putc(0xc0);
   for(int c=0; c<31; c++) {
      term_putc(0xc4);
   }
   term_putc(0xd9);
   term_putc('\n');
}

int main(char* args) {
  ansi_table();

  return 0;
}
