#ifndef SAMS_H
#define SAMS_H 1

#include "banking.h"

extern int sams_next_page;
extern int sams_total_pages;

extern volatile int sams_map_shadow[6];

// determine how many SAMS pages are available, or 0 if no SAMS detected.
int init_sams();

// set default mappings back
inline void inl_map_page(int page, int location) {
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

// map a logical page number into the target address
void map_page(int page, int addr);

// return first logical page number of allocated block of pages.
// or -1 if not enough pages are available.
int alloc_pages(int count);

// frees highest pages back to system. If you allocated 3, and free 2, you still have your first page.
// returns new top page
int free_pages(int count);

struct __attribute__((__packed__)) SamsInformation {
  int next_page;
  int total_pages;
};

// memory info
void sams_info(struct SamsInformation* info);

DECLARE_BANKED_VOID(map_page, BANK(0), bk_map_page, (int page, int addr), (page, addr))
DECLARE_BANKED(alloc_pages, BANK(0), int, bk_alloc_pages, (int count), (count))
DECLARE_BANKED(free_pages, BANK(0), int, bk_free_pages, (int count), (count))
DECLARE_BANKED_VOID(sams_info, BANK(0), bk_sams_info, (struct SamsInformation* info), (info))

#endif
