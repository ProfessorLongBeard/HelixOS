#include <pmm.h>
#include <vmm.h>



__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mm = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};


static freelist_t *head;







void pmm_init(void) {
    uint64_t hhdm_offset = vmm_get_hhdm_offset();
    struct limine_memmap_response *m = mm.response;





    for (uint32_t i = 0; i < m->entry_count; i++) {
        struct limine_memmap_entry *e = m->entries[i];

        // We only want usable regions
        if (e->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (!head) {
            // Set first node
            freelist_t *first_node = (freelist_t *)(hhdm_offset + e->base);

            first_node->base_addr = e->base;
            first_node->length = e->length;

            // Adjust base and size to account for node pointer
            first_node->base_addr += sizeof(freelist_t);
            first_node->length -= sizeof(freelist_t);
            first_node->next = NULL;

            printf("PMM: Head node: [0x%lx - 0x%lx] size = %uKB\n", first_node->base_addr, first_node->base_addr + first_node->length, first_node->length / 1024);
            head = first_node;
        } else {

            // Keep list head intact
            freelist_t *prev = head;

            // Iterate to last node
            while(prev->next != NULL) {
                prev = prev->next;
            }

            // Create new node and link it
            freelist_t *node = (freelist_t *)(hhdm_offset + e->base);

            node->base_addr = e->base;
            node->length = e->length;
            node->next = NULL;

            // Adjust base and size to account for freelist pointer
            node->base_addr += sizeof(freelist_t);
            node->length -= sizeof(freelist_t);

            // Link node
            prev->next = node;
            printf("PMM: Node region: [0x%lx - 0x%lx] size = %uKB\n", node->base_addr, node->base_addr + node->length, node->length / 1024);
        }
    }
}