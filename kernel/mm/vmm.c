#include <kstdio.h>
#include <kstring.h>
#include <kstdlib.h>
#include <limine.h>
#include <kernel.h>
#include <arch.h>
#include <pmm.h>
#include <vmm.h>





page_table_t *pgd = NULL;








void vmm_init(uint64_t kernel_phys, uint64_t kernel_virt, struct limine_memmap_entry **mm, uint64_t mm_count) {
    assert(mm != NULL && mm_count >= 1);

    if (!pgd) {
        pgd = pmm_alloc(1);
        assert(pgd != NULL);
    }

    for (uint64_t i = 0; i < mm_count; i++) {
        struct limine_memmap_entry *e = mm[i];

        if (e->type == LIMINE_MEMMAP_FRAMEBUFFER) {
            vmm_map_range(pgd, PHYS_TO_VIRT(e->base), PHYS_TO_VIRT(e->base + e->length), e->base, PT_DEVICE_RW);
            vmm_flush_cache_range(PHYS_TO_VIRT(e->base), PHYS_TO_VIRT(e->base + e->length));
        }

        if (e->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            // Re-map bootloader reclaim memory for kernel use
            vmm_map_range(pgd, PHYS_TO_VIRT(e->base), PHYS_TO_VIRT(e->base + e->length), e->base, PT_KERNEL_RW);
            vmm_flush_cache_range(PHYS_TO_VIRT(e->base), PHYS_TO_VIRT(e->base + e->length));
        }

        if (e->type == LIMINE_MEMMAP_USABLE) {
            vmm_map_range(pgd, PHYS_TO_VIRT(e->base), PHYS_TO_VIRT(e->base + e->length), e->base, PT_KERNEL_RW);
            vmm_flush_cache_range(PHYS_TO_VIRT(e->base), PHYS_TO_VIRT(e->base + e->length));
        }
    }

    // Map GIC, UART, RTC, etc
    uint64_t mmio_phys_start = 0x08000000;
    uint64_t mmio_phys_end = 0x09030000;
    uint64_t mmio_size = mmio_phys_end - mmio_phys_start;

    uint64_t mmio_virt_start = VMM_VIRT_BASE + mmio_phys_start;
    uint64_t mmio_virt_end = mmio_virt_start + mmio_size;

    vmm_map_range(pgd, mmio_virt_start, mmio_virt_end, mmio_phys_start, PT_DEVICE_RW);
    vmm_flush_cache_range(mmio_virt_start, mmio_virt_end);

    // Re-map kernel sections
    uint64_t reqs_virt_start = (uint64_t)__slimine_requests;
    uint64_t reqs_virt_end = (uint64_t)__elimine_requests;
    uint64_t reqs_phys_start = (uint64_t)kernel_phys + ((uint64_t)reqs_virt_start - (uint64_t)kernel_virt);

    vmm_map_range(pgd, reqs_virt_start, reqs_virt_end, reqs_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(reqs_virt_start, reqs_virt_end);

    uint64_t text_virt_start = (uint64_t)__stext;
    uint64_t text_virt_end = (uint64_t)__etext;
    uint64_t text_phys_start = (uint64_t)kernel_phys + ((uint64_t)text_virt_start - (uint64_t)kernel_virt);

    vmm_map_range(pgd, text_virt_start, text_virt_end, text_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(text_virt_start, text_virt_end);

    uint64_t rodata_virt_start = (uint64_t)__srodata;
    uint64_t rodata_virt_end = (uint64_t)__erodata;
    uint64_t rodata_phys_start = (uint64_t)kernel_phys + ((uint64_t)rodata_virt_start - (uint64_t)kernel_virt);

    vmm_map_range(pgd, rodata_virt_start, rodata_virt_end, rodata_phys_start, PT_KERNEL_RO);
    vmm_flush_cache_range(rodata_virt_start, rodata_virt_end);

    uint64_t data_virt_start = (uint64_t)__sdata;
    uint64_t data_virt_end = (uint64_t)__edata;
    uint64_t data_phys_start = (uint64_t)kernel_phys + (data_virt_start - kernel_virt);

    vmm_map_range(pgd, data_virt_start, data_virt_end, data_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(data_virt_start, data_virt_end);

    uint64_t bss_virt_start = (uint64_t)__sbss;
    uint64_t bss_virt_end = (uint64_t)__ebss;
    uint64_t bss_phys_start = (uint64_t)kernel_phys + (bss_virt_start - kernel_virt);

    vmm_map_range(pgd, bss_virt_start, bss_virt_end, bss_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(bss_virt_start, bss_virt_end);

    uint64_t stack_virt_start = (uint64_t)__sstack;
    uint64_t stack_virt_end = (uint64_t)__estack;    
    uint64_t stack_phys_start = (uint64_t)kernel_phys + (stack_virt_start - kernel_virt);

    vmm_map_range(pgd, stack_virt_start, stack_virt_end, stack_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(stack_virt_start, stack_virt_end);

    vmm_switch_pagemap(pgd);
    vmm_inval_all();

    printf("VMM: Initialized!\n");
}

void vmm_map_range(page_table_t *table, uint64_t virt_start, uint64_t virt_end, uint64_t phys, uint64_t flags) {
    assert(table != NULL);
    assert(virt_start < virt_end);

    uint64_t virt_start_aligned = ALIGN_DOWN(virt_start, PAGE_SIZE);
    uint64_t virt_end_aligned = ALIGN_UP(virt_end, PAGE_SIZE);

    uint64_t phys_aligned = ALIGN_DOWN(phys, PAGE_SIZE);

    uint64_t virt_size = virt_end_aligned - virt_start_aligned;
    uint64_t num_pages = SIZE_TO_PAGES(virt_size, PAGE_SIZE);

    for (uint64_t i = 0; i < num_pages; i++) {
        vmm_map(table, virt_start_aligned + (i * PAGE_SIZE), phys_aligned + (i * PAGE_SIZE), flags);
    }
}

void vmm_unmap_range(page_table_t *table, uint64_t virt_start, uint64_t virt_end, uint64_t phys_start) {
    assert(table != NULL);
    assert(virt_start < virt_end);

    uint64_t virt_size = virt_end - virt_start;
    uint64_t num_pages = SIZE_TO_PAGES(virt_size, PAGE_SIZE);

    for (uint64_t i = 0; i < num_pages; i++) {
        vmm_unmap(table, virt_start + (i * PAGE_SIZE), phys_start + (i * PAGE_SIZE));
    }
}

void vmm_map(page_table_t *table, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t l0_idx = PGD_IDX(virt);
    uint64_t l1_idx = PUD_IDX(virt);
    uint64_t l2_idx = PMD_IDX(virt);
    uint64_t l3_idx = PTE_IDX(virt);




    assert(table != NULL);
    page_table_t *l0 = (page_table_t *)PHYS_TO_VIRT((uint64_t)table);




    if (!(l0->entries[l0_idx] & PT_VALID)) {
        page_table_t *l1 = pmm_alloc(1);
        assert(l1 != NULL);

        l0->entries[l0_idx] = (uint64_t)l1 | PT_TABLE | PT_VALID;
    }

    page_table_t *l1 = (page_table_t *)PHYS_TO_VIRT(l0->entries[l0_idx] & ~0xFFF);
    
    if (!(l1->entries[l1_idx] & PT_VALID)) {
        page_table_t *l2 = (page_table_t *)pmm_alloc(1);
        assert(l2 != NULL);

        l1->entries[l1_idx] = (uint64_t)l2 | PT_TABLE | PT_VALID;
    }

    page_table_t *l2 = (page_table_t *)PHYS_TO_VIRT(l1->entries[l1_idx] & ~0xFFF);
    
    if (!(l2->entries[l2_idx] & PT_VALID)) {
        page_table_t *l3 = (page_table_t *)pmm_alloc(1);
        assert(l3 != NULL);

        l2->entries[l2_idx] = (uint64_t)l3 | PT_TABLE | PT_VALID;
    }

    page_table_t *l3 = (page_table_t *)PHYS_TO_VIRT(l2->entries[l2_idx] & ~0xFFF);

    if (!(l3->entries[l3_idx] & PT_VALID)) {
        l3->entries[l3_idx] = (uint64_t)phys | flags;
    }
}

void vmm_unmap(page_table_t *table, uint64_t virt, uint64_t phys) {
    uint64_t l0_idx = PGD_IDX(virt);
    uint64_t l1_idx = PUD_IDX(virt);
    uint64_t l2_idx = PMD_IDX(virt);
    uint64_t l3_idx = PTE_IDX(virt);


    
    assert(table != NULL);
    page_table_t *l0 = (page_table_t *)PHYS_TO_VIRT((uint64_t)table);
    page_table_t *l1 = NULL, *l2 = NULL, *l3 = NULL;

    if (l0->entries[l0_idx] & PT_VALID) {
        l1 = (page_table_t *)PHYS_TO_VIRT(l0->entries[l0_idx] & ~0xFFF);
    } else {
        return;
    }

    if (l1->entries[l1_idx] & PT_VALID) {
        l2 = (page_table_t *)PHYS_TO_VIRT(l1->entries[l1_idx] & ~0xFFF);
    } else {
        return;
    }

    if (l2->entries[l2_idx] & PT_VALID) {
        l3 = (page_table_t *)PHYS_TO_VIRT(l2->entries[l2_idx] & ~0xFFF);
    } else {
        return;
    }

    uint64_t pte = PHYS_TO_VIRT(l3->entries[l3_idx] & PT_PADDR_MASK);
    printf("pte 0x%lx\n", pte);

    l3->entries[l3_idx] = 0;
    vmm_inval_page((uint64_t)pte);
}

void vmm_inval_all(void) {
    __tlb_inval_all();
}

void vmm_inval_page(uint64_t addr) {
    __tlb_inval_page(addr);
}

void vmm_switch_pagemap(page_table_t *table) {
    __ttbr1_write((uint64_t)table);
}

void vmm_flush_dcache_addr(uint64_t addr) {
    __dcache_flush_addr((uint64_t)addr);
}

void vmm_flush_icache_addr(uint64_t addr) {
    __icache_flush_addr((uint64_t)addr);
}

void vmm_flush_cache_range(uint64_t virt_start, uint64_t virt_end) {
    __flush_cache_range((uint64_t)virt_start, (uint64_t)virt_end);
}