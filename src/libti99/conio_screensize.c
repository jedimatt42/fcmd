#include "banks.h"
#define MYBANK BANK(8)

#include "conio.h"

void screensize(unsigned char *x, unsigned char *y) {
    *x = nTextEnd - nTextRow + 1;
    *y = (nTextEnd / *x) + 1;
}
