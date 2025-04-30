#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <kernel.h>
#include <spinlock.h>




static spinlock_t s;
static heap_page_list_t *page_list;
static kslab_cache_t kslab_cache[KSLAB_COUNT];









static kslab_cache_t *kslab_cache_for_each(int slab_order) {
    if (slab_order > KSLAB_COUNT) {
        printf("HEAP: Kernel slab order %d out of bounds!\n", slab_order);
        return NULL;
    }

    return &kslab_cache[slab_order];
}

static int kslab_get_order(size_t length) {
    int slab_order = 0;
    size_t slab_size = KSLAB_MIN;

    while(slab_size <= length && slab_order <= KSLAB_COUNT - 1) {
        slab_size <<= 1;
        slab_order++;
    }

    return slab_order;
}

static size_t kslab_get_size(size_t length) {
    size_t slab_size = KSLAB_MIN;

    while(slab_size <= length) {
        slab_size <<= 1;
    }

    return slab_size;
}

void kheap_init(void) {
    void *free_list = NULL;
    size_t slab_size = KSLAB_MIN;
    kslab_t *slab = NULL;



    spinlock_init(&s);

    for (int i = 0; i <= KSLAB_COUNT && slab_size <= KSLAB_MAX; i++) {
        kslab_cache_t *cache = kslab_cache_for_each(i);
        assert(cache != NULL);

        // Allocate page frame for slab object
        slab = pmm_alloc();
        assert(slab != NULL);

        // Get physical address of page frame
        uintptr_t slab_phys_base = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)slab);
        uintptr_t slab_virt_base = HEAP_VIRT_BASE + slab_phys_base;
        uintptr_t slab_virt_end = slab_virt_base + PAGE_SIZE;
        uintptr_t slab_orig_virt_base = VMM_VIRT_BASE + slab_phys_base;

        // Map slab object into kernel heap virtual space
        vmm_map(vmm_get_pgd(), slab_virt_base, slab_phys_base, PT_KERNEL_RW | PT_HEAP);
        vmm_inval_page(slab_virt_base);

        // Unmap orignal page entry
        vmm_unmap(vmm_get_pgd(), slab_orig_virt_base, slab_phys_base);
        vmm_flush_cache_range(slab_virt_base, slab_virt_end);

        // Ensure slab object points to new mapping
        slab = (kslab_t *)slab_virt_base;

        slab->total_objects = PAGE_SIZE / slab_size;
        slab->free_objects = slab->total_objects;
        slab->object_size = slab_size + sizeof(uintptr_t);
        slab->free_list = NULL;

        free_list = (void *)((uintptr_t)slab + sizeof(kslab_t));

        for (uint64_t i = 0; i < slab->total_objects; i++) {
            if ((uintptr_t)free_list + slab->object_size > (uintptr_t)slab + PAGE_SIZE) {
                // Prevent writing outside of the mapped page
                break;
            }

            *(void **)free_list = slab->free_list;
            slab->free_list = free_list;
            free_list = (void *)((uint8_t *)free_list + slab->object_size);
        }

        slab->next = cache->slab_list;
        cache->slab_list_count = slab->total_objects;
        cache->slab_list = slab;

        slab_size <<= 1;
    }

    // Allocate initial page heap list
    page_list = kslab_alloc(sizeof(heap_page_list_t));
    assert(page_list != NULL);

    printf("HEAP: Initialized\n");
}

static kslab_t *kslab_create_new(size_t length) {
    void *free_list = NULL;
    kslab_t *slab = NULL;
    size_t slab_size = kslab_get_size(length);
    int slab_order = kslab_get_order(length);
    
    
    
    kslab_cache_t *cache = kslab_cache_for_each(slab_order);
    assert(cache != NULL);

    // Allocate page frame for slab object
    slab = pmm_alloc();
    assert(slab != NULL);

    // Get physical address of page frame
    uintptr_t slab_phys_base = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)slab);
    uintptr_t slab_virt_base = HEAP_VIRT_BASE + slab_phys_base;
    uintptr_t slab_virt_end = slab_virt_base + PAGE_SIZE;
    uintptr_t slab_orig_virt_base = VMM_VIRT_BASE + slab_phys_base;

    // Map slab object into kernel heap virtual space
    vmm_map(vmm_get_pgd(), slab_virt_base, slab_phys_base, PT_KERNEL_RW | PT_HEAP);
    vmm_inval_page(slab_virt_base);

    // Unmap orignal page entry
    vmm_unmap(vmm_get_pgd(), slab_orig_virt_base, slab_phys_base);
    vmm_flush_cache_range(slab_virt_base, slab_virt_end);

    // Ensure slab object points to new mapping
    slab = (kslab_t *)slab_virt_base;

    slab->total_objects = PAGE_SIZE / slab_size;
    slab->free_objects = slab->total_objects;
    slab->object_size = slab_size;
    slab->free_list = NULL;

    free_list = (void *)((uintptr_t)slab + sizeof(kslab_t));

    for (uint64_t i = 0; i < slab->total_objects; i++) {
        if ((uintptr_t)free_list + slab->object_size > (uintptr_t)slab + PAGE_SIZE) {
            // Prevent writing outside of the mapped page
            break;
        }

        *(void **)free_list = slab->free_list;
        slab->free_list = free_list;
        free_list = (void *)((uint8_t *)free_list + slab->object_size);
    }

    slab->next = cache->slab_list;
    cache->slab_list_count = slab->total_objects;
    cache->slab_list = slab;

    return slab;
}

static void *kslab_get_object(kslab_t *slab, size_t length) {
    void *ptr = NULL;
    kslab_t *curr = slab;
    uintptr_t *slab_ptr = NULL;



    if (!curr) {
        return NULL;
    }

    if (curr->free_objects == 0) {
        // New slab object should be created
        return NULL;
    }

    ptr = (void *)slab->free_list;

    if (!ptr) {
        return NULL;
    }

    slab->free_list = *(void **)ptr;
    slab->free_objects--;

    return ptr;
}

static kslab_t *kslab_find_first_free(size_t length) {
    size_t order = kslab_get_order(length);
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;


    cache = kslab_cache_for_each(order);

    if (!cache) {
        return NULL;
    }

    slab = cache->slab_list;

    if (!slab) {
        return NULL;
    }

    while(slab != NULL) {
        if (slab->free_objects > 0 && slab->object_size == length) {
            return slab;
        }

        slab = slab->next;
    }

    return NULL;
}

static kslab_t *kslab_get_end(kslab_t *slab) {
    kslab_t *head = slab;

    assert(head != NULL);

    while(head->next != NULL) {
        head = head->next;
    }

    return head;
}

static void kslab_link_to_cache(kslab_t *new_slab) {
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;
    size_t order = 0;
    
    
    assert(new_slab != NULL);
    
    order = kslab_get_order(new_slab->object_size);
    
    if (order > KSLAB_COUNT) {
        return;
    }
    
    cache = kslab_cache_for_each(order);
    
    if (!cache) {
        return;
    }

    if (cache->slab_list == NULL) {
        cache->slab_list = new_slab;
    } else {
        kslab_t *next = kslab_get_end(cache->slab_list);
        
        if (!next) {
            return;
        }

        next->next = new_slab;
    }

    cache->slab_list_count++;
}

void *kslab_alloc(size_t length) {
    void *ptr = NULL;
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;
    int slab_order = 0;
    


    if (length < KSLAB_MIN) {
        length = KSLAB_MIN;
    }

    slab_order = kslab_get_order(length);

    if (slab_order > KSLAB_COUNT) {
        return NULL;
    }

    cache = kslab_cache_for_each(slab_order);

    if (!cache) {
        return NULL;
    }

    slab = cache->slab_list;

    if (!slab) {
        return NULL;
    }

    // TODO: Maybe check if slab is NULL instead of slab->next?
    while(slab->next != NULL) {
        if (slab->free_objects > 0 && slab->object_size >= length) {
            break;
        }

        slab = slab->next;
    }

    if (slab->free_objects == 0) {
        slab = kslab_create_new(length);

        if (!slab) {
            return NULL;
        }

        kslab_link_to_cache(slab);

        ptr = kslab_get_object(slab, length);

        if (!ptr) {
            return NULL;
        }

        memset(ptr, 0, length);
        return ptr;
    }

    ptr = kslab_get_object(slab, length);

    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, length);
    return ptr;
}

void kslab_free(void *ptr, size_t length) {
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;
    int slab_order = kslab_get_order(length);
    size_t slab_size = kslab_get_size(length);
 


    cache = kslab_cache_for_each(slab_order);

    if (!cache) {
        return;
    }

    slab = cache->slab_list;

    while(slab != NULL) {
        uintptr_t slab_start = (uintptr_t)slab + sizeof(*slab);
        uintptr_t slab_end = slab_start + (slab->total_objects * slab->object_size);

        if ((uintptr_t)ptr >= slab_start && (uintptr_t)ptr < slab_end) {
            // Place freed object back into freelist
            *(void **)ptr = slab->free_list;
            slab->free_list = (void *)ptr;
            slab->free_objects++;
        }

        // Move to next slab in list
        slab = slab->next;
    }
}

static heap_page_list_t *heap_find_list(heap_page_list_t *list, uintptr_t phys_base, uintptr_t virt_base, size_t length) {
    heap_page_list_t *curr = list;



    if (!list) {
        return NULL;
    }

    while(curr != NULL) {
        // Look for page node to recycle for use in list
        if (curr->virt_base == virt_base && curr->phys_base == phys_base && curr->length == length) {
            return curr;
        }

        // If none were found go to the next node
        curr = curr->next;
    }

    return NULL;
}

static heap_page_list_t *heap_list_alloc(void) {
    heap_page_list_t *new_list = NULL;


    new_list = kslab_alloc(sizeof(heap_page_list_t));

    if (!new_list) {
        return NULL;
    }

    memset(new_list, 0, sizeof(heap_page_list_t));

    return new_list;
}

static heap_page_list_t *heap_list_get_end(void) {
    heap_page_list_t *curr = page_list;

    if (!curr) {
        return NULL;
    }

    while(curr->next != NULL) {
        curr = curr->next;
    }

    return curr;
}

static void heap_push_to_list(heap_page_list_t *list) {
    heap_page_list_t *curr = NULL;


    if (!list) {
        return;
    }

    curr = heap_list_get_end();

    if (!curr) {
        return;
    }

    curr->next = list;
}

static heap_page_list_t *heap_remove_from_list(heap_page_list_t **list, uintptr_t phys) {
    heap_page_list_t *curr = *list;
    heap_page_list_t *prev = NULL;


    if (!list) {
        return NULL;
    }

    while(curr) {
        if (curr->phys_base == phys) {
            if (prev) {
                prev->next = curr->next;
            } else {
                *list = curr->next;
            }

            // Ensure to free used slab object
            kslab_free(curr->next, sizeof(heap_page_list_t));

            curr->next = NULL;
            return curr;
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

static void *heap_alloc_from_list(heap_page_list_t *list) {
    void *ptr = NULL;

    
    if (!list) {
        return NULL;
    }

    // Get pointer to virtual base
    ptr = (void *)list->virt_base;

    if (!ptr) {
        return NULL;
    }

    // Remove this page node from the list to prevent it being allocated a 2nd time
    heap_remove_from_list(&page_list, list->phys_base);

    return ptr;
}

void *kmalloc(size_t length) {
    void *ptr = NULL, *tmp_ptr = NULL;
    heap_page_list_t *page = NULL;
    size_t page_count = 0;
    kslab_t *slab = NULL;



    // DON'T FORGET TO RELEASE THE FUCKING LOCK!!!
    spinlock_acquire(&s);

    if (length <= KSLAB_MAX) {
        // If requested allocation is within the supported slab size grab a slab from cache
        ptr = kslab_alloc(length);

        if (!ptr) {
            // Assume (for now) that we've run out of available slabs, so just create a new one
            slab = kslab_create_new(length);

            if (!slab) {
                spinlock_release(&s);
                return NULL;
            }

            // Link new slab object to existing cache
            kslab_link_to_cache(slab);

            // Get object from new slab
            ptr = kslab_get_object(slab, length);

            if (!ptr) {
                spinlock_release(&s);
                return NULL;
            }

            spinlock_release(&s);
            return ptr;
        }

        spinlock_release(&s);
        return ptr;
    }

    uintptr_t phys_base = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)ptr);
    uintptr_t virt_base = HEAP_VIRT_BASE + phys_base;
    uintptr_t virt_end = virt_base + length;
    uintptr_t orig_virt_base = VMM_VIRT_BASE + phys_base;
    uintptr_t orig_virt_end = orig_virt_base + length;

    page = heap_find_list(page_list, phys_base, virt_base, length);

    if (!page) {
        // Create new page(s) and link them to list
        page = heap_list_alloc();

        if (!page) {
            spinlock_release(&s);
            return NULL;
        }

        page->length = length;
        page->page_count = SIZE_TO_PAGES(length, PAGE_SIZE);

        // Request page frames from the PMM
        tmp_ptr = pmm_alloc_pages(page->page_count);

        if (!tmp_ptr) {
            kslab_free(page, sizeof(heap_page_list_t));

            spinlock_release(&s);
            return NULL;
        }

        // Get physical base of page frame
        page->phys_base = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)tmp_ptr);
        page->virt_base = HEAP_VIRT_BASE + page->phys_base;

        orig_virt_base = VMM_VIRT_BASE + page->phys_base;
        orig_virt_end = orig_virt_base + page->length;

        // Create new mapping for heap
        vmm_map_range(vmm_get_pgd(), page->virt_base, page->virt_base + page->length, page->phys_base, PT_KERNEL_RW | PT_HEAP);
        vmm_inval_page_range(page->virt_base, page->length);

        // Unmap previous HHDM mapping
        vmm_unmap(vmm_get_pgd(), orig_virt_base, phys_base);
        vmm_flush_cache_range(orig_virt_base, orig_virt_end);

        // Insert page into free list of pages
        heap_push_to_list(page);
    }

    // Allocate existing page from list
    ptr = heap_alloc_from_list(page);

    if (!ptr) {
        spinlock_release(&s);
        return NULL;
    }

    spinlock_release(&s);
    return ptr;
}

void kfree(void *ptr, size_t length) {
    heap_page_list_t *page = NULL;


    spinlock_acquire(&s);

    if (!ptr) {
        spinlock_release(&s);
        return;
    }

    if (length <= KSLAB_MAX) {
        kslab_free(ptr, length);

        spinlock_release(&s);
        return;
    }

    uintptr_t phys_base = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)ptr);
    uintptr_t virt_base = HEAP_VIRT_BASE + phys_base;
    uintptr_t virt_end = virt_base + length;

    page = heap_list_alloc();

    if (!page) {
        spinlock_release(&s);
        return;
    }

    page->phys_base = phys_base;
    page->virt_base = virt_base;
    page->length = length;
    page->page_count = SIZE_TO_PAGES(page->length, PAGE_SIZE);

    // Push page node back into free list
    heap_push_to_list(page);

    spinlock_release(&s);
}