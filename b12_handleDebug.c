#include "banks.h"
#define MYBANK BANK(12)

#include "commands.h"

#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b8_terminal.h"
#include "b8_getstr.h"
#include <string.h>

void dumpMem(unsigned int addr, int len);

void cmdM(char* tok) {
    int limit = bk_atoi(bk_strtok(0, ' '));
    dumpMem(bk_htoi(tok + 1), limit == 0 ? 8 : limit);
}


void handleDebug() {
    char cmdline[40];
    while (1) {
        for(int i=0; i<40; i++) {cmdline[i] = 0;}
        tputs_rom("\n] ");
        bk_getstr(cmdline, 40, 1);
        bk_tputc('\n');

        char* tok = bk_strtok(cmdline, ' ');
        int toklen = bk_strlen(tok);
        if (toklen > 0) {
            char cmd = tok[0];
            switch(cmd) {
                case 'M':
                    cmdM(tok);
                    break;
                case 'Q':
                    return;
                default:
                    tputs_rom("unkown command\n");
            }
        }
    }
}

void dumpMem(unsigned int addr, int len) {
    unsigned int* memory = (unsigned int*)addr;
    unsigned int* limit = (int*)addr + len;
    while(memory < limit && request_break == 0) {
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