#include <kstdio.h>
#include <kstdlib.h>
#include <pmm.h>
#include <vmm.h>





__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mm = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static freelist_t *free_mem = NULL;
static freelist_t *used_mem = NULL;














static freelist_t *pmm_get_last_node(freelist_t *head) {
    freelist_t *prev = head;

    assert(prev != NULL);

    while(prev->next != NULL) {
        prev = prev->next;
    }

    return prev;
}

static void pmm_push_node(freelist_t *head, freelist_t *node) {
    freelist_t *prev = NULL;

    assert(head != NULL);
    assert(node != NULL);

    prev = pmm_get_last_node(head);
    assert(prev != NULL);

    prev->next = node;
}

static void pmm_pop_node(freelist_t *head, freelist_t *node) {
    freelist_t *prev = head;

    assert(prev != NULL);
    assert(node != NULL);

    while(prev->next != NULL) {
        if (prev->phys_base == node->phys_base && prev->phys_end == node->phys_end && prev->length == node->length) {
            // Remove this node from the list, and stich the others together
        }

        prev = prev->next;
    }
}

static void pmm_pop_last_node(freelist_t *head) {
    freelist_t *prev = head;

    assert(prev != NULL);

    while(prev->next && prev->next->next) {
        prev = prev->next;
    }

    prev->next = NULL;
}

void pmm_init(void) {
    struct limine_memmap_response *m = mm.response;
    uint64_t hhdm_offset = vmm_get_hhdm_offset();
    freelist_t *head = NULL;

    

    


    for (uint32_t i = 0; i < m->entry_count; i++) {
        struct limine_memmap_entry *e = m->entries[i];

        // We only want usable regions
        if (e->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (!free_mem) {
            freelist_t *first_node = (freelist_t *)(hhdm_offset + e->base);
            first_node->phys_base = e->base;
            first_node->phys_end = e->base + e->length;
            first_node->length = e->length;
            first_node->next = NULL;

            free_mem = first_node;
            head = free_mem;

            printf("PMM: Available region: [0x%lx - 0x%lx] region size: %uKB\n", first_node->phys_base, first_node->phys_end, first_node->length / 1024);
        
        } else {
            freelist_t *node = (freelist_t *)(hhdm_offset + e->base);
            node->phys_base = e->base;
            node->phys_end = e->base + e->length;
            node->length = e->length;
            node->next = NULL;

            pmm_push_node(free_mem, node);
            printf("PMM: Available region: [0x%lx - 0x%lx] region size: %uKB\n", node->phys_base, node->phys_end, node->length / 1024);
        }
    }

    // Ensure first list entry is used for allocations
    free_mem = head;
    printf("PMM: Set head node: [0x%lx - 0x%lx] region size: %uKB\n", head->phys_base, head->phys_end, head->length / 1024);
}

void *pmm_alloc(void) {
    freelist_t *head = free_mem, *page_head = used_mem;
    freelist_t *page = NULL;
    uint64_t hhdm_offset = vmm_get_hhdm_offset();
    void *ptr = NULL;



    assert(head != NULL);

    page = (freelist_t *)(hhdm_offset + head->phys_base);
    page->phys_base = head->phys_base;
    page->phys_end = page->phys_base + PMM_PAGE_SIZE;
    page->length = PMM_PAGE_SIZE;
    page->next = NULL;

    if (!used_mem) {
        used_mem = page;

        ptr = (void *)(hhdm_offset + page->phys_base);
        printf("PMM: Allocated page: [0x%lx - 0x%lx] page size: %uKB\n", page->phys_base, page->phys_end, page->length / 1024);

        head->phys_base += PMM_PAGE_SIZE;
        head->length -= PMM_PAGE_SIZE;
        return ptr;
    }

    ptr = (void *)(hhdm_offset + page->phys_base);
    printf("PMM: Allocated page: [0x%lx - 0x%lx] page size: %uKB\n", page->phys_base, page->phys_end, page->length / 1024);

    head->phys_base += PMM_PAGE_SIZE;
    head->length -= PMM_PAGE_SIZE;

    pmm_push_node(used_mem, page);
    return ptr;
}

void pmm_free(void *ptr) {
}