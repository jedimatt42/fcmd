#include "banks.h"
#define MYBANK BANK(7)

#include "gpu_scroll.h"
#include "f18a_gpu.h"
#include <vdp.h>

// conio_bgcolor.c
extern unsigned int conio_scrnCol;

void gpu_scroll_80color() {
    // This routine is installed only for the 80x30 mode, so there are 29
    // scrollable rows above the bottom row.
    const int line = nTextEnd - nTextRow + 1;
    const int first = nTitleLine ? 1 : 0;
    const int rows = 29 - first;

    // Scroll the text and attribute rows up with the built-in BLKCPY routine.
    f18a_gpu_blkcpy(gImage + (first + 1) * line, gImage + first * line,
                    rows * line);
    f18a_gpu_blkfill(gImage + nTextRow, ' ', line);
    f18a_gpu_blkcpy(gColor + (first + 1) * line, gColor + first * line,
                    rows * line);
    f18a_gpu_blkfill(gColor + nTextRow, (int)conio_scrnCol, line);
}
