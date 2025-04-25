#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm.h>
#include <kernel.h>
#include <spinlock.h>




static spinlock_t s;
static heap_info_t  heap_info;
static heap_block_t *heap_head = NULL;








void heap_init(struct limine_memmap_response *m) {
    uintptr_t pmm_usable_base = pmm_get_phys_base();
    uintptr_t pmm_usable_end = pmm_get_phys_end();
    uintptr_t pmm_usable_size = pmm_get_phys_size();




    spinlock_init(&s);

    for (uint32_t i = 0; i < m->entry_count; i++) {
        struct limine_memmap_entry *e = m->entries[i];

        if (e->type == LIMINE_MEMMAP_USABLE || e->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE || e->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE) {
            uintptr_t base = e->base;
            uintptr_t end = e->base + e->length;
            size_t size = e->length;

            // Get the next usable region outside of the PMM usable memory region
            if (base == pmm_usable_base && end <= pmm_usable_end) {
                continue;
            }

            // Use the next entry available to us
            if (heap_info.heap_phys_base == 0) {
                heap_info.heap_phys_base = base;
            }

            heap_info.heap_size += size;
        }
    }

    heap_info.heap_virt_base = HEAP_VIRT_BASE;
    heap_info.heap_phys_end = heap_info.heap_phys_base + heap_info.heap_size;
    heap_info.heap_virt_end = heap_info.heap_virt_base + heap_info.heap_size;

    printf("HEAP: Available region: 0x%lx - 0x%lx length: %luMB\n", heap_info.heap_phys_base, heap_info.heap_phys_end, heap_info.heap_size / (1024 * 1024));
    printf("HEAP: Virtual region: 0x%lx - 0x%lx length: %luMB\n", heap_info.heap_virt_base, heap_info.heap_virt_end, heap_info.heap_size / (1024 * 1024));

    // Map kernel heap 
    vmm_map_range(vmm_get_pgd(), heap_info.heap_virt_base, heap_info.heap_virt_end, heap_info.heap_phys_base, PT_KERNEL_RW | PT_HEAP);
    vmm_flush_cache_range(heap_info.heap_virt_base, heap_info.heap_virt_end);

    heap_head = (heap_block_t *)heap_info.heap_virt_base;
    heap_head->magic = HEAP_MAGIC;
    heap_head->length = heap_info.heap_size;
    heap_head->is_free = true;
    heap_head->next = NULL;

    printf("HEAP: Initialized!\n");
}

static heap_block_t *heap_find_first_free(size_t length) {
    heap_block_t *curr = heap_head;

    while(curr != NULL) {
        if (curr->is_free == true && curr->length >= length) {
            return curr;
        }

        curr = curr->next;
    }

    return NULL;
}

static void heap_split(heap_block_t *ptr, size_t length) {
    heap_block_t *block = NULL;
    size_t remaining = 0, total_req_length = 0;



    if (!ptr) {
        return;
    }

    total_req_length = length + sizeof(heap_block_t);

    if (ptr->length >= total_req_length + sizeof(heap_block_t))  {
        printf("HEAP: Splitting block: 0x%lx length: %lu\n", (uintptr_t)ptr, length);
        remaining = ptr->length - total_req_length;

        heap_block_t *new_block = (heap_block_t *)((uintptr_t)ptr + total_req_length);
        new_block->length = remaining;
        new_block->is_free = true;
        new_block->is_aligned = false;
        new_block->next = ptr->next;

        ptr->magic = HEAP_MAGIC;
        ptr->length = length;
        ptr->is_free = false;
        ptr->is_aligned = false;
        ptr->next = new_block;
    } else {
        printf("HEAP: Using whole block: 0x%lx, length: %lu\n", (uintptr_t)ptr, length);
        ptr->is_free = false;
    }
}

static void heap_expand(void) {
    // TODO: Implement heap expansion
}

void *kmalloc(size_t length) {
    void *ptr = NULL;
    heap_block_t *new_block = NULL;


    spinlock_acquire(&s);

    new_block = heap_find_first_free(length);

    if (!new_block) {
        printf("HEAP: Failed to allocate length %lu! (out of memory)\n", length);

        spinlock_release(&s);
        return NULL;
    }

    heap_split(new_block, length);
    assert(new_block != NULL);

    ptr = (void *)((uintptr_t)new_block + sizeof(heap_block_t));
    assert(ptr != NULL);

    // Ensure allocated block is cleared
    memset(ptr, 0, length);

    printf("HEAP: Allocated block: 0x%lx length: %lu\n", (uintptr_t)ptr, length);

    spinlock_release(&s);
    return ptr;
}

void kfree(void *ptr) {
    heap_block_t *curr = heap_head, *block = NULL;


    spinlock_acquire(&s);

    if (!ptr) {
        spinlock_release(&s);
        return;
    }

    block = (heap_block_t *)((uintptr_t)ptr - sizeof(heap_block_t));
    assert(block != NULL);

    if (block->magic != HEAP_MAGIC) {
        printf("HEAP: Invalid heap magic 0x%lx! (corrupted)\n", block->magic);
        return;
    }

    if (block->is_free == true) {
        printf("HEAP: Double free detected @ 0x%lx!\n", (uintptr_t)ptr);
        return;
    }

    if (block->is_aligned == true) {
        printf("HEAP: Freeing aligned block: 0x%lx length: %lu\n", (uintptr_t)ptr, block->length);
        block->is_aligned = false;
        block->is_free = true;
    } else {
        printf("HEAP: Freeing block: 0x%lx length: %lu\n", (uintptr_t)ptr, block->length);
        block->is_free = true;
    }

    while(curr && curr->next) {
        // Coalesce adjacent free blocks in attempt to prevent fragmentation
        if (curr->is_free == true && curr->next->is_free == true) {
            curr->length += sizeof(heap_block_t) + curr->next->length;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }

    spinlock_release(&s);
}

void *kmalloc_aligned(size_t length, uint64_t alignment) {
    void *ptr = NULL;
    uintptr_t start = 0, aligned_start = 0;
    heap_block_t *curr = heap_head, *new_block = NULL;
    size_t padding = 0;



    spinlock_acquire(&s);

    new_block = heap_find_first_free(length + alignment);

    if (!new_block) {
        printf("HEAP: Failed to allocate length %lu! (out of memory)\n", length);

        spinlock_release(&s);
        return NULL;
    }

    start = (uintptr_t)new_block + sizeof(heap_block_t);
    aligned_start = ALIGN_UP(start, alignment);
    padding = aligned_start - start;

    if (padding > 0) {
        // Split to skip padding
        heap_split(new_block, padding);
        new_block = new_block->next;
    }

    heap_split(new_block, length);
    new_block->is_aligned = true;
    new_block->is_free = false;

    ptr = (void *)((uintptr_t)new_block + sizeof(heap_block_t));
    assert(ptr != NULL);

    memset(ptr, 0, length);

    printf("HEAP: Allocated aligned block: 0x%lx length: %lu\n", (uintptr_t)ptr, length);

    spinlock_release(&s);
    return ptr;
}