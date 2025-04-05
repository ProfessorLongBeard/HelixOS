#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <spinlock.h>



#define MAX_SLABS   10

typedef struct {
    spinlock_t      s;
    uint64_t        hhdm;
    uint64_t        free_pages;
    uint64_t        used_pages;
    uint64_t        last_used_idx;

    uint64_t        bitmap_base;
    uint64_t        bitmap_end;
    uint64_t        bitmap_size;
    uint8_t         *bitmap;
} pmm_t;






void pmm_init(struct limine_memmap_entry **mm, uint64_t mm_count);
void *pmm_alloc(uint64_t page_count);
void pmm_free(void *ptr, uint64_t page_count);

#endif