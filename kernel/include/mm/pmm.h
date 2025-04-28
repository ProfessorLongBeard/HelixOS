#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <spinlock.h>



#define BITS_PER_BYTE   8



typedef struct {
    spinlock_t  s;

    uintptr_t   phys_start;
    uintptr_t   phys_end;
    size_t      phys_size;

    uintptr_t   bitmap_base;
    uintptr_t   bitmap_end;
    size_t      bitmap_size;

    size_t      total_pages;
    size_t      used_pages;
    size_t      reserved_pages;

    uint8_t     *bitmap;
} bitmap_t;


void pmm_init(struct limine_memmap_response *m);
void *pmm_alloc(void);
void pmm_free(void *ptr);
void *pmm_alloc_pages(size_t page_count);
void pmm_free_pages(void *ptr, size_t page_count);
uintptr_t pmm_get_phys_base(void);
uintptr_t pmm_get_phys_end(void);
size_t pmm_get_phys_size(void);

#endif