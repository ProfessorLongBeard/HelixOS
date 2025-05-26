#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>





// Virtual address of kernel heap mappings
#define HEAP_VIRT_BASE  0xFFFF800000000000



#define KSLAB_COUNT     20
#define KSLAB_MIN       8
#define KSLAB_MAX       2048



typedef struct kslab {
    uint64_t        total_objects;      // Total objects in slab
    uint64_t        free_objects;       // Available slab objects
    size_t          object_size;        // Size of this slab object
    void            *free_list;         // Pointer to slab freelist
    struct kslab    *next;              // Next slab object in list
} kslab_t;

typedef struct kslab_cache {
    uint64_t    slab_list_count;        // Number of slab lists in cache region
    kslab_t     *slab_list;             // Slab list in cache
} kslab_cache_t;

typedef struct heap_page_list {
    uintptr_t               phys_base;  // Physical address of mapped page
    uintptr_t               virt_base;  // Virtual address of mapped page
    uint64_t                page_count; // Number of allocated pages
    size_t                  length;     // Length of page range
    struct heap_page_list   *next;      // Next page(s) allocation in list
} heap_page_list_t;







void kheap_init(void);
void *kslab_alloc(size_t length);
void kslab_free(void *ptr, size_t length);
void *kmalloc(size_t length);
void kfree(void *ptr, size_t length);
void *krealloc(void *old_ptr, size_t old_length, size_t new_length);

#endif