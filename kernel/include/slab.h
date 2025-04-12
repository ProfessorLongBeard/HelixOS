#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <stddef.h>




#define SLAB_MIN_SIZE   8       // 8 bytes
#define SLAB_MAX_SIZE   4096
#define SLAB_COUNT      20


void *slab_alloc(size_t length);
void slab_free(void *ptr, size_t length);
typedef struct slab {
    uint64_t        total_objects;  // Number of objects within a slab
    uint64_t        free_objects;   // Number of free objects within a slab
    uint64_t        used_objects;   // Number of used objects within a slan 
    size_t          object_size;    // Object size of slab
    void            *free_list;     // Pointer to slab object
    struct slab     *next;          // Next slab in list
} slab_t;

typedef struct slab_cache {
    uint64_t    object_count;       // Number of slab objects
    slab_t      *slab_list;         // Linked list of slab objects
} slab_cache_t;




void slab_init(void);
void *slab_alloc(size_t length);
void slab_free(void *ptr, size_t length);

#endif