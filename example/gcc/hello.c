#include "fc_api.h"

int main(char* args) {
   fc_tputs("Args: ");
   fc_tputs(args);

   fc_tputs("\nHello, what is your name? ");
   char name[14];
   for(int i = 0; i<14; i++) {
      name[i] = 0;
   }
   fc_getstr(name, 13, 1);
   fc_tputs("\nHello ");
   fc_tputs(name);
   fc_tputs("!\nNice to meet you.\n");
   return 0;
}
