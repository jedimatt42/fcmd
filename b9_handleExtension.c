#include "banking.h"
#define MYBANK BANK(9)

#include "b8_terminal.h"
#include "commands.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include <vdp.h>

int loadExtension(const char* ext);

void handleExtension(const char* ext) {
    // TODO: search path for command
    // for-now: just look in TIPI.FC.BIN

    int err = loadExtension(ext);

    if (err) {
        tputs_rom("unknown command: ");
        bk_tputs_ram(ext);
        bk_tputc('\n');
        return;
    }

    int* arg1 = (int*) ext + bk_strlen(ext) + 1;
    __asm__(
        "mov @>A006,r0\n\t"
        "mov %1,r1\n\t"
        "bl *r0\n\t"
        "mov r1,%0\n\t"
        : "=r" (err)   /* output list */
        : "r" (arg1)   /* input list */
        : "r0", "r1"   /* register clobber list */
    );

    if (err) {
        tputs_rom("error result: ");
        bk_tputs_ram(bk_uint2str(err));
        bk_tputc('\n');
    }
}

int loadExtension(const char* ext) {
    struct DeviceServiceRoutine *dsr = 0;
    char path[256];
    char* cpuAddr = (char*) 0xA000;

    {
        char fullname[256];
        bk_strcpy(fullname, str2ram("DSK1."));
        bk_strcat(fullname, ext);
        bk_parsePathParam(fullname, &dsr, path, PR_REQUIRED);
        if (!dsr) {
            return 1;
        }
    }

    unsigned int unit = bk_path2unitmask(path);

    int parent_idx = bk_lindexof(path, '.', bk_strlen(path) - 1);
    char filename[11];
    bk_strncpy(filename, path + parent_idx + 1, 10);
    path[parent_idx + 1] = 0x00;

    bk_lvl2_setdir(dsr->crubase, unit, path);

    // AddInfo must be in scratchpad
    struct AddInfo *addInfoPtr = (struct AddInfo *)0x8320;
    addInfoPtr->first_sector = 0;
    addInfoPtr->eof_offset = 0;
    addInfoPtr->flags = 0;
    addInfoPtr->rec_length = 0;
    addInfoPtr->records = 0;
    addInfoPtr->recs_per_sec = 0;

    unsigned int err = bk_lvl2_input(dsr->crubase, unit, filename, 0, addInfoPtr);
    if (err)
    {
        return 1;
    }

    // TODO : Check the type is PROGRAM

    unsigned char eof_offset = addInfoPtr->eof_offset;

    int totalBlocks = addInfoPtr->first_sector;
    if (totalBlocks == 0)
    {
        tputs_rom("error, file is empty.\n");
        return 1;
    }
    // Allow the file to be larger, but only load the first 24K.
    if (totalBlocks > 96) {
        totalBlocks = 96;
    }

    int blockId = 0;
    while (blockId < totalBlocks)
    {
        addInfoPtr->first_sector = blockId;
        err = bk_lvl2_input(dsr->crubase, unit, filename, 1, addInfoPtr);
        if (err)
        {
            tputs_rom("error reading file: ");
            bk_tputs_ram(bk_uint2hex(err));
            bk_tputc('\n');
            return 1;
        }

        // TODO: If first block, check that it meets header requirements

        vdpmemread(addInfoPtr->buffer, cpuAddr, 256);
        cpuAddr += 256;

        blockId++;
    }
    return 0;
}