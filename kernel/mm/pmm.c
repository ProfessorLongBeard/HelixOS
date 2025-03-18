#include <kstdio.h>
#include <kstdlib.h>
#include <pmm.h>
#include <vmm.h>





__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mm = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};



static freelist_t   *free;
static page_node_t  *used;














void pmm_init(void) {
    uint64_t hhdm_offset = vmm_get_hhdm_offset();
    struct limine_memmap_response *m = mm.response;
    freelist_t *head = NULL;




    for (uint32_t i = 0; i < m->entry_count; i++) {
        struct limine_memmap_entry *e = m->entries[i];

        // We only want usable regions
        if (e->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        // Create freelist nodes of available usable memory regions
        freelist_t *node = (freelist_t *)(hhdm_offset + e->base);
        node->base = e->base;
        node->end = e->base + e->length;
        node->size = e->length;
        node->next = NULL;

        printf("PMM: Free region: [0x%lx - 0x%lx] size = %uKB\n", node->base, node->end, node->size / 1024);

        if (!free) {
            // Set head node
            free = node;
            head = free;

        } else {
            // Get end of list
            while(free->next != NULL) {
                free = free->next;
            }

            // Link node
            free->next = node;
        }
    }

    // Ensure first usable region is set first
    free = head;
}

void *pmm_alloc(void) {
    freelist_t *curr_free = free;
    page_node_t *curr_page = used;
    uint64_t hhdm_offset = vmm_get_hhdm_offset();
    void *ptr = NULL;





    if (!used) {
        page_node_t *first_page = (page_node_t *)(hhdm_offset + curr_free->base);
        first_page->base = curr_free->base;
        first_page->size = PMM_PAGE_SIZE;
        first_page->next = NULL;

        ptr = (void *)(hhdm_offset + first_page->base);

        curr_free->base += PMM_PAGE_SIZE;
        used = first_page;

        printf("PMM: Allocated page: [0x%lx - 0x%lx] size = %uKB\n", first_page->base, first_page->base + first_page->size, first_page->size / 1024);
    } else {
        // Iterate to last linked page
        while(curr_page->next != NULL) {
            curr_page = curr_page->next;
        }

        // Ensure we're onto the next page
        curr_free->base += PMM_PAGE_SIZE;

        page_node_t *page = (page_node_t *)(hhdm_offset + curr_free->base);
        page->base = curr_free->base;
        page->size = PMM_PAGE_SIZE;

        ptr = (void *)(hhdm_offset + page->base);

        curr_page->next = page;
        printf("PMM: Allocated page: [0x%lx - 0x%lx] size = %uKB\n", page->base, page->base + page->size, page->size / 1024);
    }

    return ptr;
}