#ifndef MM_H
#define MM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>






#define PAGE_SIZE   4096

#define ALIGN_UP(base, align) ((base + (align - 1)) & ~(align - 1))
#define ALIGN_DOWN(base, align) (base & ~(align - 1))

#define SIZE_TO_PAGES(size, page_size) ((size + (page_size - 1)) / page_size)









void mm_init(void);
uint32_t mm_get_num_entries(void);
struct limine_memmap_entry *mm_entry_for_each(uint32_t idx);
uint64_t mm_get_hhdm_offset(void);
struct limine_memmap_entry *mm_get_entry_by_type(uint32_t type);

#endif