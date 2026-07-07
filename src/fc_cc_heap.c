#include "banks.h"
#define MYBANK BANK(0)

#include "sams.h"

#define PAGE_SIZE 4096
#define MIN_BLOCK_SIZE 4
#define BLOCK_HEADER_SIZE 2
#define MAX_HEAP_PAGES 256

// Per-page free list entry (embedded on each SAMS heap page)
// Block header at offset `off`:
//   word 0: [alloc:1][size:15]  — block size includes the 2-byte header
// Free blocks only, at offset `off+2`:
//   word 1: next_free offset within page (-1 = end of list)

// Always-resident page table
static struct {
    int page_id;    // SAMS page number
    int free_head;  // offset of first free block within page (-1 = page full)
} page_table[MAX_HEAP_PAGES];
static int page_count;

// Local pool of pages returned by free() that are entirely empty
static int free_page_pool[MAX_HEAP_PAGES];
static int free_page_count;

// Find page table index for a given page_id, or -1 if not found
static int find_page(int page_id) {
    for (int i = 0; i < page_count; i++) {
        if (page_table[i].page_id == page_id)
            return i;
    }
    return -1;
}

// Read a word from a mapped SAMS page window
static int rw(int addr) {
    return *(volatile int*)addr;
}

// Write a word to a mapped SAMS page window
static void ww(int addr, int val) {
    *(volatile int*)addr = val;
}

int fc_cc_malloc(int size, int* out) {
    if (size <= 0 || size > PAGE_SIZE - BLOCK_HEADER_SIZE)
        return -1;

    int block_size = (size + BLOCK_HEADER_SIZE + 1) & ~1;
    if (block_size < MIN_BLOCK_SIZE)
        block_size = MIN_BLOCK_SIZE;

    // Phase 1: search existing pages
    for (int i = 0; i < page_count; i++) {
        int free_head = page_table[i].free_head;
        if (free_head == -1)
            continue;

        sams_map_page(page_table[i].page_id, 0xE000);

        int prev = -1;
        int curr = free_head;
        while (curr != -1) {
            int header = rw(0xE000 + curr);
            int blk_size = header & 0x7FFF;

            if (blk_size >= block_size) {
                int remaining = blk_size - block_size;

                if (remaining >= MIN_BLOCK_SIZE) {
                    int old_next = rw(0xE000 + curr + 2);
                    ww(0xE000 + curr, (1 << 15) | block_size);
                    ww(0xE000 + curr + block_size, remaining);
                    ww(0xE000 + curr + block_size + 2, old_next);
                    int new_free = curr + block_size;
                    if (prev == -1)
                        page_table[i].free_head = new_free;
                    else
                        ww(0xE000 + prev + 2, new_free);
                } else {
                    ww(0xE000 + curr, (1 << 15) | blk_size);
                    int next = rw(0xE000 + curr + 2);
                    if (prev == -1)
                        page_table[i].free_head = next;
                    else
                        ww(0xE000 + prev + 2, next);
                }

                out[0] = curr + BLOCK_HEADER_SIZE;
                out[1] = page_table[i].page_id;
                return 0;
            }

            prev = curr;
            curr = rw(0xE000 + curr + 2);
        }
    }

    // Phase 2: allocate a new page
    int page_id;
    if (free_page_count > 0) {
        page_id = free_page_pool[--free_page_count];
    } else {
        page_id = sams_alloc_pages(1);
        if (page_id < 0)
            return -1;
    }

    sams_map_page(page_id, 0xE000);

    int remaining = PAGE_SIZE - block_size;
    ww(0xE000, (1 << 15) | block_size);

    if (remaining >= MIN_BLOCK_SIZE) {
        ww(0xE000 + block_size, remaining);
        ww(0xE000 + block_size + 2, -1);
        page_table[page_count].free_head = block_size;
    } else {
        page_table[page_count].free_head = -1;
    }

    page_table[page_count].page_id = page_id;
    page_count++;

    out[0] = BLOCK_HEADER_SIZE;
    out[1] = page_id;
    return 0;
}

int fc_cc_free(int* ptr) {
    int data_offset = ptr[0] & 0x0FFF;
    int header_off = data_offset - BLOCK_HEADER_SIZE;
    int page_id = ptr[1];

    if (header_off >= PAGE_SIZE || header_off < 0)
        return -1;

    int pi = find_page(page_id);
    if (pi < 0)
        return -1;

    sams_map_page(page_id, 0xE000);

    int header = rw(0xE000 + header_off);
    if (!(header & 0x8000))
        return -1;

    int block_size = header & 0x7FFF;

    // Clear the allocated flag
    ww(0xE000 + header_off, block_size);

    // Coalesce with next block if free
    int next_off = header_off + block_size;
    int merged_size = block_size;
    if (next_off < PAGE_SIZE) {
        int nh = rw(0xE000 + next_off);
        if (!(nh & 0x8000)) {
            merged_size += nh & 0x7FFF;
            int nn = rw(0xE000 + next_off + 2);
            ww(0xE000 + header_off, merged_size);
            ww(0xE000 + header_off + 2, nn);
        } else {
            ww(0xE000 + header_off + 2, page_table[pi].free_head);
        }
    } else {
        ww(0xE000 + header_off + 2, page_table[pi].free_head);
    }

    page_table[pi].free_head = header_off;

    // Zero the caller's handle
    ptr[0] = 0;
    ptr[1] = 0;

    return 0;
}

int fc_cc_calloc(int count, int size, int* out) {
    int total = count * size;
    if (fc_cc_malloc(total, out) != 0)
        return -1;

    int data_offset = out[0] & 0x0FFF;
    int page_id = out[1];

    sams_map_page(page_id, 0xE000);
    int block_size = rw(0xE000 + data_offset - BLOCK_HEADER_SIZE) & 0x7FFF;
    int data_size = block_size - BLOCK_HEADER_SIZE;

    for (int i = 0; i < data_size; i += 2)
        ww(0xE000 + data_offset + i, 0);
    if (data_size & 1)
        *(volatile char*)(0xE000 + data_offset + data_size - 1) = 0;

    return 0;
}
