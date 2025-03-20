#include <kstdio.h>
#include <kstdlib.h>
#include <stdbool.h>
#include <mm.h>




static freelist_t *free_mem;
static freelist_t *used_mem;

static size_t total_mem = 0;
static size_t allocated_mem = 0;








void pmm_init(void) {
    uint64_t hhdm_offset = 0;
    uint32_t mm_entries = 0;
    freelist_t *head = NULL;




    mm_entries = mm_get_num_entries();
    assert(mm_entries > 0);

    hhdm_offset = mm_get_hhdm_offset();
    assert(hhdm_offset > 0);

    for (uint32_t i = 0; i < mm_entries; i++) {
        struct limine_memmap_entry *e = mm_entry_for_each(i);

        if (e->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        total_mem += e->length;

        freelist_t *free_node = (freelist_t *)(hhdm_offset + e->base);
        free_node->base = e->base;
        free_node->size = e->length;
        free_node->end = free_node->base + free_node->size;
        free_node->next = NULL;

        printf("PMM: Available region: [0x%lx - 0x%lx] region size: %uKB\n", free_node->base, free_node->end, free_node->size / 1024);

        if (!free_mem) {
            free_mem = free_node;
            head = free_node;

        } else {
            while(free_mem->next != NULL) {
                free_mem = free_mem->next;
            }

            free_mem->next = free_node;
        }
    }

    free_mem = head;
}

void *pmm_alloc(void) {
    freelist_t *page = NULL;
    uint64_t hhdm_offset = mm_get_hhdm_offset();


    assert(free_mem != NULL);

    if (allocated_mem >= total_mem)  {
        printf("PMM: Switching regions...\n");
    }

    page = (freelist_t *)(hhdm_offset + free_mem->base);
    page->base = hhdm_offset + free_mem->base;
    page->size = PAGE_SIZE;
    page->end = page->base + page->size;
    page->next = NULL;
    void *ptr = (void *)page->base;

    if (!used_mem) {
        used_mem = page;
    } else {
        while(used_mem->next != NULL) {
            used_mem = used_mem->next;
        }

        used_mem->next = page;
    }

    printf("PMM: Allocated page: [0x%lx - 0x%lx] page size %uKB\n", page->base, page->end, page->size / 1024);

    free_mem->base += PAGE_SIZE;
    allocated_mem += PAGE_SIZE;

    return ptr;
}