#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>



// 16MB intiial heap space
#define HEAP_INITIAL_SIZE   (16 * 1024 * 1024)

// Reserved virtual memory region for kernel heap
#define HEAP_VIRT_BASE      0xFFFF800000000000
#define HEAP_VIRT_END       (HEAP_VIRT_BASE + HEAP_INITIAL_SIZE)





typedef struct heap_block {
    uintptr_t           base;
    size_t              length;
    bool                is_free;
    struct heap_block   *next;
} heap_block_t;




void heap_init(uintptr_t kernel_phys, uintptr_t kernel_virt);
void *kmalloc(size_t length);
void kfree(void *ptr);

#endif