#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>




// Magic identifier to verify heap wasn't corrupted
#define HEAP_MAGIC  0xDEADB33F

// 16MB intiial heap space (TODO: expand when needed)
//#define HEAP_INITIAL_SIZE   (16 * 1024 * 1024)

// Reserved virtual memory region for kernel heap
#define HEAP_VIRT_BASE      0xFFFF800000000000
//#define HEAP_VIRT_END       (HEAP_VIRT_BASE + HEAP_INITIAL_SIZE)



typedef struct {
    uintptr_t   heap_phys_base;
    uintptr_t   heap_phys_end;
    size_t      heap_size;

    uintptr_t   heap_virt_base;
    uintptr_t   heap_virt_end;
} heap_info_t;

typedef struct heap_block {
    uint64_t            magic;
    size_t              length;
    bool                is_free;
    bool                is_aligned;
    struct heap_block   *next;
} heap_block_t;







void heap_init(struct limine_memmap_response *m);
void *kmalloc(size_t length);
void kfree(void *ptr);
void *kmalloc_aligned(size_t length, uint64_t alignment);

#endif