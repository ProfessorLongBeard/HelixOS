#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm.h>
#include <kernel.h>
#include <spinlock.h>




static spinlock_t s;
static heap_block_t *heap_head = (heap_block_t *)__kernel_end;







void heap_init(uintptr_t kernel_phys, uintptr_t kernel_virt) {
    uintptr_t heap_virt_base = HEAP_VIRT_BASE;
    uintptr_t heap_virt_end = HEAP_VIRT_END;
    uintptr_t heap_phys_base = (uintptr_t)kernel_phys + ((uintptr_t)heap_head - kernel_virt);
    size_t heap_size = HEAP_INITIAL_SIZE;



    spinlock_init(&s);

    printf("HEAP: Available heap region: 0x%lx - 0x%lx length: %luMB\n", heap_phys_base, heap_phys_base + heap_size, heap_size / (1024 * 1024));
    printf("HEAP: Virtual region: 0x%lx - 0x%lx\n", heap_virt_base, heap_virt_end);

    vmm_map_range(vmm_get_pgd(), heap_virt_base, heap_virt_end, heap_phys_base, PT_KERNEL_RW | PT_CONTIG);
    vmm_flush_cache_range(heap_virt_base, heap_virt_end);

    heap_head = (heap_block_t *)HEAP_VIRT_BASE;

    heap_head->base = HEAP_VIRT_BASE;
    heap_head->length = HEAP_INITIAL_SIZE;
    heap_head->is_free = true;
    heap_head->next = NULL;

    printf("HEAP: Initialized!\n");
}

static heap_block_t *heap_find_first_free(size_t length) {
    heap_block_t *curr = heap_head;

    while(curr != NULL) {
        if (curr->is_free && curr->length >= length) {
            return curr;
        }

        curr = curr->next;
    }

    return NULL;
}

static void heap_split(heap_block_t *ptr, size_t length) {
    heap_block_t *block = NULL;


    if (!ptr) {
        return;
    }

    block = (heap_block_t *)((uintptr_t)ptr + sizeof(heap_block_t) + length);
    assert(block != NULL);

    block->base = (uintptr_t)block;
    block->length = ptr->length - length - sizeof(heap_block_t);
    block->is_free = true;

    ptr->base = (uintptr_t)block;
    ptr->length = length;
    ptr->is_free = false;
    ptr->next = block;
}

void *kmalloc(size_t length) {
    void *ptr = NULL;
    heap_block_t *block = NULL;



    spinlock_acquire(&s);

    if (length > HEAP_INITIAL_SIZE) {
        // Ensure we don't attempt to allocate out of the heap range
        printf("HEAP: Length %lu out of range of kernel heap space!\n", length / (1024 * 1024));

        spinlock_release(&s);
        return NULL;
    }

    block = heap_find_first_free(length);

    if (!block) {
        printf("HEAP: Failed to allocate length %lu! (out of memory)\n", length);
        return NULL;
    }

    heap_split(block, length);

    ptr = (void *)((uintptr_t)block + sizeof(heap_block_t));
    assert(ptr != NULL);

    // Ensure clean allocation
    memset(ptr, 0, length);

    printf("HEAP: Allocated: %lu @ 0x%lx\n", block->length, block->base);

    spinlock_release(&s);
    return ptr;
}

void kfree(void *ptr) {
    heap_block_t *block = NULL, *curr = heap_head;


    spinlock_acquire(&s);

    if (!ptr) {
        spinlock_release(&s);
        return;
    }

    // Get heap block metadata
    block = (heap_block_t *)((uintptr_t)ptr - sizeof(heap_block_t));
    assert(block != NULL);

    if (block->is_free) {
        printf("HEAP: Double free detected @ 0x%lx\n", block->base);

        spinlock_release(&s);
        return;
    }

    printf("HEAP: Freeing 0x%lx of length %lu\n", block->base, block->length);

    block->is_free = true;

    while(curr && curr->next) {
        // Coalesce adjacent free blocks in attempt to prevent fragmentation
        if (curr->is_free && curr->next->is_free) {
            curr->length += sizeof(heap_block_t) + curr->next->length;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }

    spinlock_release(&s);
}