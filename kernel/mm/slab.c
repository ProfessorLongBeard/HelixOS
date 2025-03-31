#include <kstdio.h>
#include <kstdlib.h>
#include <mm.h>




/*
 * Slab allocation method:
 * - Find first free object from slab_t in slab_cache
 * - Take a free object from the freelist
 * - Update the freelist pointer
 * - Decrement the freelist object count and return the allocated
 */

 /*
  * Slab allocation:
  * - Start at the proper slab based on the cache size
  * - Walk the slab list to look for free objects; If found, return and object for it
  *
  * If none are found:
  * - Request a new slab (pmm_alloc)
  * - Initialize the freelist
  * - add it to the slab cache
  * - Return first free object
  */

/*
 * Slab order count in powers of two
 * 8 bytes
 * 16 bytes
 * 32 bytes
 * 64 bytes
 * 128 bytes
 * 256 bytes
 * 512 bytes
 * 1024 bytes
 * 2048 bytes
 * 4096 bytes
 */






static spinlock_t s;
static slab_cache_t slab_cache[SLAB_COUNT];







static slab_cache_t *slab_cache_for_each(size_t slab_order) {
    slab_cache_t *cache = NULL;

    cache = &slab_cache[slab_order];
    assert(cache != NULL);

    return cache;
}

static size_t slab_get_order(size_t length) {
    size_t order = 0, order_aligned = 0;

    order_aligned = ALIGN_UP(length, PAGE_SIZE);

    while(order_aligned > SLAB_MIN_SIZE) {
        order_aligned >>= 1;
        order++;
    }

    return order;
}

void slab_init(void) {
    void *ptr = NULL;
    size_t order_count = SLAB_MIN_SIZE;
    spinlock_init(&s);



    for (size_t i = 0; i < SLAB_COUNT && order_count <= SLAB_MAX_SIZE; i++) {
        slab_cache_t *cache = slab_cache_for_each(i);

        slab_t *slab = pmm_alloc();
        assert(slab != NULL);

        slab->num_objects = PAGE_SIZE / SLAB_MIN_SIZE;
        slab->free_objects = slab->num_objects;
        slab->object_size = SLAB_MIN_SIZE;    
        ptr = (void *)(slab + 1);

        for (size_t i = 0; i < slab->num_objects - 1; i++) {
            *(void **)ptr = slab->data;
            slab->data = ptr;
            ptr = (void *)((uint8_t *)ptr + slab->object_size);
        }

        slab->next = cache->slab_list;
        cache->num_objects = slab->num_objects;
        cache->slab_list = slab;

        // Increase slab order count
        order_count <<= 1;
    }

    printf("SLAB: Initialized!\n");
}

static slab_t *slab_create_new(size_t length) {
    void *ptr = NULL;
    slab_t *new_slab = pmm_alloc();
    size_t order = SLAB_MIN_SIZE;

    assert(new_slab != NULL);

    new_slab->num_objects = length / PAGE_SIZE;
    new_slab->free_objects = new_slab->num_objects;
    new_slab->object_size = length;
    new_slab->next = NULL;

    ptr = (void *)(new_slab + 1);
    assert(ptr != NULL);

    for (size_t i = 0; i < new_slab->num_objects - 1; i++) {
        *(void **)ptr = new_slab->data;
        new_slab->data = ptr;
        ptr = (void *)((uint8_t *)ptr + new_slab->object_size);
    }

    return new_slab;
}

static void *slab_get_obj(slab_t *slab, size_t length) {
    void *ptr = NULL;
    slab_t *curr = slab;

    assert(curr != NULL);
    assert(length > 0 && length <= PAGE_SIZE);

    if (curr->free_objects == 0) {
        printf("No available slab objects in region: 0x%lx!\n", (uintptr_t)slab);
        return NULL;
    }

    // TODO: bounds checking with length

    ptr = (void *)slab->data;
    assert(ptr != NULL);

    slab->data = *(void **)ptr;

    slab->free_objects--;
    return ptr;
}

static slab_t *slab_find_first_free(size_t length) {
    size_t order = slab_get_order(length);
    slab_t *slab = NULL;
    slab_cache_t *cache = NULL;


    cache = slab_cache_for_each(order);
    assert(cache != NULL);

    slab = cache->slab_list;
    assert(slab != NULL);

    while(slab) {
        if (slab->free_objects > 0 && slab->object_size == length) {
            return slab;
        }

        slab = slab->next;
    }

    return NULL;
}

static slab_t *slab_get_next(slab_t *slab) {
    slab_t *head = slab;
    assert(head != NULL);

    if (head->next != NULL) {
        head = head->next;
    }

    return head;
}

static slab_t *slab_get_end(slab_t *slab) {
    slab_t *head = slab;

    assert(head != NULL);

    while(head->next != NULL) {
        head = head->next;
    }

    return head;
}

static void slab_link_to_cache(slab_t *new_slab) {
    slab_t *slab = NULL;
    slab_cache_t *cache = NULL;
    size_t order = 0;
    
    
    assert(new_slab != NULL);
    
    order = slab_get_order(new_slab->object_size);
    cache = slab_cache_for_each(order);
    assert(cache != NULL);

    if (cache->slab_list == NULL) {
        cache->slab_list = new_slab;
    } else {
        slab_t *next = slab_get_end(cache->slab_list);
        assert(next != NULL);

        next->next = new_slab;
    }

    cache->num_objects++;
}

void *slab_alloc(size_t length) {
    void *ptr = NULL;
    slab_t *slab = NULL;


    assert(length <= PAGE_SIZE && length > 0);
    spinlock_acquire(&s);

    slab = slab_find_first_free(length);
    
    if (!slab) {
        slab = slab_create_new(length);
        assert(slab != NULL);

        slab_link_to_cache(slab);

        ptr = slab_get_obj(slab, length);
        assert(ptr != NULL);

        return ptr;
    }

    ptr = slab_get_obj(slab, length);
    assert(ptr != NULL);

    spinlock_release(&s);
    return ptr;
}

size_t slab_find_ptr_obj_size(void *ptr) {
    size_t order = SLAB_MIN_SIZE;
    slab_t *slab = NULL;
    slab_cache_t *cache = NULL;

    assert(ptr != NULL);


    for (size_t i = 0; i < SLAB_COUNT && order <= SLAB_MAX_SIZE; i++) {
        cache = slab_cache_for_each(i);
        assert(cache != NULL);

        slab = cache->slab_list;
        
        while(slab != NULL) {
            size_t slab_size = slab->num_objects * slab->object_size;
            uintptr_t data_start = (uintptr_t)(slab + 1);
            uintptr_t data_end = data_start + slab_size;

            if ((uintptr_t)ptr >= data_start && (uintptr_t)ptr < data_end) {
                for (uintptr_t obj = data_start; obj < data_end; obj += slab->object_size) {
                    if ((uintptr_t)ptr == obj) {
                        return slab->object_size;
                    }
                }
            }

            slab = slab->next;
        }
    }

    return 0;
}

void slab_free(void *obj, size_t length) {
    slab_t *slab = NULL;
    slab_cache_t *cache = NULL;
    size_t order = 0;

    assert(obj != NULL);
    assert(length <= PAGE_SIZE || length > 0);

    spinlock_acquire(&s);

    order = slab_get_order(length);

    cache = slab_cache_for_each(order);
    assert(cache != NULL);

    assert(cache->slab_list != NULL);
    slab = cache->slab_list;

    size_t slab_size = slab->num_objects * slab->object_size;
    uintptr_t data_start = (uintptr_t)(slab + 1);
    uintptr_t data_end = data_start + slab_size;

    uintptr_t obj_start = (uintptr_t)obj;
    uintptr_t obj_end = obj_start + length;

    size_t obj_idx = (obj_start - data_start) / slab->object_size;
    uintptr_t base = data_start + obj_idx * slab->object_size;

    if (obj_start != base) {
        printf("Failed to locate object: 0x%lx in slab!\n", (uintptr_t)obj_start);
        
        spinlock_release(&s);
        return;
    }

    if (slab->data != NULL) {
        *(void **)obj = slab->data;
        slab->free_objects++;
    }

    slab->data = obj;
    slab->free_objects++;

    if (slab->free_objects == slab->num_objects) {
        pmm_free(slab);
    }

    spinlock_release(&s);
}
