#include "banks.h"
#define MYBANK BANK(12)

#include "commands.h"

#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b8_terminal.h"
#include <string.h>

void dumpMem(unsigned int addr, int len);


void handleDebug() {
    char* tok = bk_strtok(0, ' ');
    int toklen = bk_strlen(tok);
    if (toklen == 5) {
        char cmd = tok[0];
        switch(cmd) {
            case 'C':
            case 'c':
                dumpMem(bk_htoi(tok+1), 16);
                break;
            default:
                tputs_rom("unkown command\n");
        }
    }
}

void dumpMem(unsigned int addr, int len) {
    unsigned int* memory = (unsigned int*)addr;
    unsigned int* limit = (int*)addr + len;
    while(memory < limit) {
        bk_tputs_ram(bk_uint2hex((unsigned int)memory));
        bk_tputc(':');
        for(int i=0; i<4; i++) {
            bk_tputs_ram(bk_uint2hex(*memory));
            bk_tputc(' ');
            memory++;
        }
        bk_tputc('\n');
    }
}