#include <pmm.h>
#include <vmm.h>



__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mm = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};


static freelist_t *fl;






void pmm_init(void) {
    uint64_t hhdm_offset = vmm_get_hhdm_offset();
    struct limine_memmap_response *m = mm.response;





    for (uint32_t i = 0; i < m->entry_count; i++) {
        struct limine_memmap_entry *e = m->entries[i];

        if (e->type != LIMINE_MEMMAP_USABLE && e->type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            continue;
        }

        // Get first free usable region
        if (!fl && e->type == LIMINE_MEMMAP_USABLE) {
            // NOTE: In kerenl space we must use the HHDM offset to access physical memory
            fl = (freelist_t *)(hhdm_offset + e->base);
            fl->next = NULL;

            printf("PMM: Freelist initial base: [0x%lx - 0x%lx], size = %u\n", e->base, e->base + sizeof(freelist_t), sizeof(freelist_t));

            freelist_t *first_node = (freelist_t *)(hhdm_offset + e->base + sizeof(freelist_t));
            first_node->base_addr = e->base + sizeof(freelist_t);
            first_node->length = e->length - sizeof(freelist_t);
            first_node->next = NULL;

            // Set first node
            fl->next = first_node;

        }
        // Iterate memory map and fill in next nodes
        freelist_t *node = (freelist_t *)(hhdm_offset + e->base);
        node->base_addr = e->base;
        node->length = e->length;
        node->next = NULL;

        printf("PMM: New node region: [0x%lx - 0x%lx], size = %uKB\n", node->base_addr, node->base_addr + node->length, node->length / 1024);

        node->next = fl->next;
        fl = node;
    }
}

static void *__pmm_alloc_chunk(size_t len) {
    freelist_t *curr = fl;
    void *ptr = NULL;
    uint64_t hhdm_offset = vmm_get_hhdm_offset();


    ptr = (void *)(hhdm_offset + curr->base_addr);

    curr->base_addr += len;
    curr->length -= len;

    printf("PMM: Allocated chunk: base = 0x%lx, end = 0x%lx, size = %u\n", curr->base_addr, curr->base_addr + len, len);
    return ptr;
}

static void __pmm_free_chunk(void *ptr, size_t len) {
}

void *pmm_alloc(size_t len) {
    freelist_t *curr_node = fl;
    freelist_t *prev = NULL;
    uint64_t hhdm_offset = vmm_get_hhdm_offset();




    if (len < curr_node->length) {
        void *ptr = __pmm_alloc_chunk(len);

        if (!ptr) {
            printf("PMM: Failed to allocate chunk size: %u!\n", len);
            return NULL;
        }

        return ptr;
    }

    if (len > curr_node->length) {
        // Iterate nodes till we can satisfy the allocation request
        while(curr_node != NULL && len > curr_node->length) {
            prev = curr_node;
            curr_node = curr_node->next;
        }

        // Found new node to satisfy the allocation request
        void *ptr = __pmm_alloc_chunk(len);

        if (!ptr) {
            printf("PMM: Failed to allocate chunk size: %u\n", len);
            return NULL;
        }

        return ptr;
    }

    printf("PMM: No available memory for allocation!\n");
    return NULL;
}