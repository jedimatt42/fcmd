#include "banks.h"
#define MYBANK BANK(0)

#include "b0_sams.h"
#include <vdp.h>

int sams_next_page;
int sams_total_pages;

void samsMapOn()
{
    __asm__(
        "LI r12, >1E00\n\t"
        "SBO 1\n\t");
}

void samsMapOff()
{
    __asm__(
        "LI r12, >1E00\n\t"
        "SBZ 1\n\t");
}

void __attribute__((noinline)) map_page(int page, int location)
{
    __asm__(
        "LI r12, >1E00\n\t"
        "SRL %0, 12\n\t"
        "SLA %0, 1\n\t"
        "SWPB %1\n\t"
        "SBO 0\n\t"
        "MOV %1, @>4000(%0)\n\t"
        "SBZ 0\n\t"
        "SWPB %1\n\t"
        :
        : "r"(location), "r"(page)
        : "r12");
}

int hasSams()
{
    volatile int *lower_exp = (volatile int *)0x2000;
    samsMapOn();
    map_page(0, 0x2000);
    *lower_exp = 0x1234;
    map_page(1, 0x2000);
    *lower_exp = 0;
    map_page(0, 0x2000);
    int detected = (*lower_exp == 0x1234);
    samsMapOff();
    return detected;
}

int samsPageCount()
{
    samsMapOn();
    volatile int *lower_exp = (volatile int *)0x2000;

    // set initial state of all pages
    for (int i = 0x20; i < 0x4000; i <<= 1)
    {
        map_page(i, 0x2000);
        *lower_exp = 0x1234;
    }
    // now mark pages and stop when they repeat
    int pages = 0x20;
    map_page(pages, 0x2000);
    while (pages < 0x4000 && *lower_exp != 0xFFFF)
    {
        *lower_exp = 0xFFFF;
        pages <<= 1;
        map_page(pages, 0x2000);
    }
    samsMapOff();
    return pages >> 1;
}

void check_exp_mem()
{
    // in case we have no expansion memory at all..
    volatile int *low_mem = (int *)0x2000;
    *low_mem = 0xFCFC;
    if (*low_mem != 0xFCFC)
    {
        vdpmemcpy(0x0166, "NO MEMORY EXPANSION", 19);
        while (1)
        {
            /* spin */
        }
    }
}

int init_sams() {
    check_exp_mem();

    if (hasSams()) {
        int total_pages = samsPageCount();
        samsMapOn(); // and leave it on.
        map_page(0, 0x2000);
        map_page(1, 0x3000);
        map_page(2, 0xA000);
        map_page(3, 0xB000);
        map_page(4, 0xC000);
        map_page(5, 0xD000);
        map_page(6, 0xE000);
        map_page(7, 0xF000);
        sams_next_page = 8;
        sams_total_pages = total_pages;
        return sams_total_pages;
    } else {
        return 0;
    }
}

int alloc_pages(int count) {
    int start = sams_next_page;
    if ((count + sams_next_page) < sams_total_pages) {
        sams_next_page += count;
        return start;
    }
    return -1;
}
