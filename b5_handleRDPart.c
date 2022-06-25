#include "banks.h"
#define MYBANK BANK(5)

#include "commands.h"
#include "b0_globals.h"
#include "b10_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_tifloat.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include <string.h>
#include <vdp.h>


// must hold in CPU RAM the name of the subroutine
#define FACADDR *((volatile int*)0x834A)

// hold VDP address of basic_str for subroutine name ( immediately followed by params )
#define PADDR *((volatile int*)0x832C)

// hold VDP address of parameters to subroutine ( should be value of 832C + namelen + 1
#define PARAMS *((volatile int*)0x8356)

#define BTOK_PARAM_LIST 0xB7
#define BTOK_PARAM_SEP 0xB3
#define BTOK_PARAM_END 0xB6
#define BTOK_NUM_STR 0xC8
#define BTOK_QUOTED_STR 0xC7

static int encodeLiteralInt(char* buf, int value) {
    char* intstr = bk_uint2str(value);
    int is_len = bk_strlen(intstr);
    int i = 0;
    buf[i++] = BTOK_NUM_STR;
    buf[i++] = (char) is_len;
    bk_strncpy(buf+i, intstr, 5);
    return is_len + 2;
}

static int encodeQuotedString(char* buf, char* str) {
    int is_len = bk_strlen(str);
    int i = 0;
    buf[i++] = BTOK_QUOTED_STR;
    buf[i++] = (char) is_len;
    bk_strncpy(buf+i, str, 10);
    return is_len + 2;
}

static int findCrubase() {
    struct DeviceServiceRoutine* dsr = bk_findDsr(str2ram("RD"), 0);
    if (!dsr) {
        tputs_rom("RD device not found\n");
        return 0;
    }
    return dsr->crubase;
}

static void call_part(int crubase, int ram, int disk, int spool) {
    // setup parameters -
    // PADDR <- FBUF: (VDP) 04 "PART" B7 C8 02 33 32 B3 C8 03 34 30 30 B3 C8 02 38 30 B6
    //                                PL NS   '3''2' PS NS   '4''0''0' PS NS   '8''0' PE
    // 0x834A <- "PART"
    // 0x8356 <- FBUF + 5
    PADDR = FBUF;
    PARAMS = FBUF+5;
    bk_strncpy((char*)FACADDR, str2ram("PART"), 4);

    // build FBUF up
    char command[30];
    bk_strset(command, 0, 30);
    int i = 0;
    command[i++] = 4;
    bk_strcpy(command+i, str2ram("PART"));
    i += 4;
    command[i++] = BTOK_PARAM_LIST;
    i += encodeLiteralInt(&(command[i]), ram);
    command[i++] = BTOK_PARAM_SEP;
    i += encodeLiteralInt(&(command[i]), disk);
    command[i++] = BTOK_PARAM_SEP;
    i += encodeLiteralInt(&(command[i]), spool);
    command[i++] = BTOK_PARAM_END;

    // copy command string to VDP as if tokenized in BASIC
    vdpmemcpy(FBUF, command, 30);

    // call basic routine
    bk_call_basic_sub(crubase, str2ram("PART"));
}

static void call_emdk(int crubase, int drive) {
    // setup parameters -
    // PADDR <- FBUF: (VDP) 04 "EMDK" B7 C8 01 32 B6
    //                                PL NS   '2' PE
    // 0x834A <- "EMDK"
    // 0x8356 <- FBUF + 5
    PADDR = FBUF;
    PARAMS = FBUF+5;
    bk_strncpy((char*)FACADDR, str2ram("EMDK"), 4);

    // build FBUF up
    char command[30];
    bk_strset(command, 0, 30);
    int i = 0;
    command[i++] = 4;
    bk_strcpy(command+i, str2ram("EMDK"));
    i += 4;
    command[i++] = BTOK_PARAM_LIST;
    i += encodeLiteralInt(&(command[i]), drive);
    command[i++] = BTOK_PARAM_END;

    // copy command string to VDP as if tokenized in BASIC
    vdpmemcpy(FBUF, command, 30);

    // call basic routine
    bk_call_basic_sub(crubase, str2ram("EMDK"));
}

static void call_vol(int crubase, char* name) {
    // setup parameters -
    // PADDR <- FBUF: (VDP) 04 "VOL" B7 C8 01 32 B6
    //                               PL NS   '2' PE
    // 0x834A <- "VOL"
    // 0x8356 <- FBUF + 4
    PADDR = FBUF;
    PARAMS = FBUF+4;
    bk_strncpy((char*)FACADDR, str2ram("VOL"), 3);

    // build FBUF up
    char command[30];
    bk_strset(command, 0, 30);
    int i = 0;
    command[i++] = 3;
    bk_strcpy(command+i, str2ram("VOL"));
    i += 3;
    command[i++] = BTOK_PARAM_LIST;
    // encode the basic string parameter
    // todoo...  
    i += encodeQuotedString(&(command[i]), name);

    command[i++] = BTOK_PARAM_END;

    // copy command string to VDP as if tokenized in BASIC
    vdpmemcpy(FBUF, command, 30);

    // call basic routine
    bk_call_basic_sub(crubase, str2ram("VOL"));
}

void handleRDPart() {
    //  RDPART
    //  RDPART <ram> <drive> <spool>

    // test for Myarc RD
    int crubase = findCrubase();
    if (!crubase) {
        return;
    }

    int ram = bk_atoi(bk_strtok(0, ' '));
    if (ram != 32 && ram != 128) {
        tputs_rom("ram allocation must be 32 or 128\n");
        return;
    }

    int disk = bk_atoi(bk_strtok(0, ' '));
    if (disk < 0) {
        tputs_rom("disk allocation must be >= 0\n");
        return;
    }

    int spool = bk_atoi(bk_strtok(0, ' '));
    if (spool < 0) {
        tputs_rom("spool allocation must be >= 0\n");
        return;
    }

    int sum = ram + disk + spool;
    if (sum != 128 && sum != 512) {
        tputs_rom("total allocation must match card size\n");
        return;
    }

    call_part(crubase, ram, disk, spool);
}

void handleRDEmdk() {
    //  RDEMDK
    //  RDEMDK <drive>

    // test for Myarc RD
    int crubase = findCrubase();
    if (crubase) {
        int drive = bk_atoi(bk_strtok(0, ' '));
        if (drive < 0 && drive > 5) {
            tputs_rom("drive must be 0 - 5\n");
            return;
        }
        call_emdk(crubase, drive);
    }
}

void handleRDVol() {
    //  RDVOL
    //  RDVOL name

    // test for Myarc RD
    int crubase = findCrubase();
    if (crubase) {
        char* vol = bk_strtok(0, ' ');
        if (!vol) {
            tputs_rom("volume name required\n");
            return;
        }
        call_vol(crubase, vol);
    }
}
