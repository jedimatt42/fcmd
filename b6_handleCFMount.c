#include "banks.h"
#define MYBANK BANK_6

#include "commands.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b1cp_terminal.h"
#include <string.h>
#include <vdp.h>

/*
 *  VDP Address | Purpose
 *  ----------- | ----------------------------
 *  0x3FF8-9    | if 0xAA03 then CF exists
 *  0x3FFA-B    | volume# of DSK1
 *  0x3FFC-D    | volume# of DSK2
 *  0x3FFE-F    | volume# of DSK3
 */

static void listAllVolumes() {

}

static void listCurrentVolumes() {
    int mapping[3];
    vdpmemread(0x3FFA, (unsigned char*)mapping, 6);
    for (int i=0; i<3; i++) {
        tputs_rom("DSK");
        tputs_ram(int2str(i + 1));
        tputs_rom(". -> ");
        tputs_ram(int2str(mapping[i]));
        tputc('\n');
    }
}

static void call_mount(int drive, int volume) {

}

static void tmp_mount(int drive, int volume) {
    int base_addr = 0x3FF8;
    base_addr += (2*drive);
    VDP_SET_ADDRESS_WRITE(base_addr);
    VDPWD = volume >> 8;
    VDPWD = volume & 0x00FF;
}


void handleCFMount() {
    //  CFMOUNT
    //  CFMOUNT [/v]
    //  CFMOUNT [/p] DSKn. vol
    char *peek = strtokpeek(0, " ");
    int list = peek == 0;
    int volumes = 0 == strcmpi("/v", peek);
    int persist = 0 == strcmpi("/p", peek);

    // test for CF7/nanopeb
    VDP_SET_ADDRESS(0x3FF8);
    int cf_nano_flag = 0;
    cf_nano_flag = VDPRD;
    cf_nano_flag = (cf_nano_flag << 8) + VDPRD;

    if (cf_nano_flag != 0xAA03) {
        tputs_rom("no CF7 or Nanopeb found\n");
        return;
    }

    if (list) {
        listCurrentVolumes();
        return;
    }

    if (volumes) {
        listAllVolumes();
        return;
    }

    if (persist)
    {
        strtok(0, " "); // consume the optional /c
    }

    int drive = atoi(strtok(0, " "));
    if (drive < 1 || drive > 3) {
        tputs_rom("drive number must be 1, 2 or 3\n");
        return;
    }

    int volume = atoi(strtok(0, " "));
    if (volume == 0) {
        tputs_rom("volume number must be > 0\n");
        return;
    }

    if (persist) {
        call_mount(drive, volume);
    } else {
        tmp_mount(drive, volume);
    }
}
