#include "banks.h"
#define MYBANK BANK(8)

#include "conio.h"

void cclearxy(int col, int row, int v) {
    gotoxy(col,row); 
    cclear(v);
}

