#include <fc_api.h>

int hexdigit(int v) {
   if (v > 9) {
      return v + 'A' - 10;
   } else {
      return v + '0';
   }
}

void ansi_table() {
   fc_tputs("  ");
   for(int c=0; c<16; c++) {
      fc_tputc(hexdigit(c));
      fc_tputc(' ');
   }
   fc_tputc('\n');
   fc_tputc(' ');
   fc_tputc(0xd5);
   for(int c=0; c<31; c++) {
      fc_tputc(0xcd);
   }
   fc_tputc(0xb8);
   fc_tputc('\n');
   for(int r=0; r<16; r++) {
      fc_tputc(hexdigit(r));
      fc_tputc(0xb3);
      for(int c=0; c<16; c++) {
         int value = (r << 4) + c;
         if (value < ' ') {
           fc_tputc(27);
         }
         fc_tputc(value);
         if (c < 15) {
           fc_tputc(' ');
         }
      }
      fc_tputc(0xb3);
      fc_tputc('\n');
   }
   fc_tputc(' ');
   fc_tputc(0xc0);
   for(int c=0; c<31; c++) {
      fc_tputc(0xc4);
   }
   fc_tputc(0xd9);
   fc_tputc('\n');
}

int fc_main(char* args) {
  ansi_table();

  return 0;
}
