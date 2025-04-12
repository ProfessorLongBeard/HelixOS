#include <kstdio.h>
#include <kstring.h>
#include <kstdlib.h>
#include <limine.h>
#include <kernel.h>
#include <arch.h>
#include <mm.h>





static spinlock_t s;
static page_table_t *pgd = NULL;








void vmm_init(uintptr_t kernel_phys, uintptr_t kernel_virt, struct limine_memmap_response *m) {
    assert(m != NULL);

    spinlock_init(&s);

    if (!pgd) {
        pgd = pmm_alloc();
        assert(pgd != NULL);
    }

    for (uint64_t i = 0; i < m->entry_count; i++) {
        struct limine_memmap_entry *e = m->entries[i];

        if (e->type == LIMINE_MEMMAP_FRAMEBUFFER) {
            uintptr_t fb_phys_base = e->base;
            uintptr_t fb_virt_base = (uintptr_t)VMM_VIRT_BASE + fb_phys_base;
            uintptr_t fb_virt_end = fb_virt_base + e->length;

            vmm_map_range(pgd, fb_virt_base, fb_virt_end, fb_phys_base, PT_DEVICE_RW);
            vmm_flush_cache_range(fb_virt_base, fb_virt_end);
        }

        if (e->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            uintptr_t reclaim_phys_base = e->base;
            uintptr_t reclaim_virt_base = (uintptr_t)VMM_VIRT_BASE + reclaim_phys_base;
            uintptr_t reclaim_virt_end = reclaim_virt_base + e->length;

            vmm_map_range(pgd, reclaim_virt_base, reclaim_virt_end, reclaim_phys_base, PT_KERNEL_RW);
            vmm_flush_cache_range(reclaim_virt_base, reclaim_virt_end);
        }

        if (e->type == LIMINE_MEMMAP_USABLE) {
            uintptr_t mem_phys_base = e->base;
            uintptr_t mem_virt_base = (uintptr_t)VMM_VIRT_BASE + mem_phys_base;
            uintptr_t mem_virt_end = mem_virt_base + e->length;

            vmm_map_range(pgd, mem_virt_base, mem_virt_end, mem_phys_base, PT_KERNEL_RW);
            vmm_flush_cache_range(mem_virt_base, mem_virt_end);
        }
    }

    // Map virtio-mmio
    uintptr_t virtio_mmio_phys_start = 0x0A000000;
    uintptr_t virtio_mmio_virt_start = (uintptr_t)VMM_VIRT_BASE + virtio_mmio_phys_start;
    uintptr_t virtio_mmio_virt_end = virtio_mmio_virt_start + PAGE_SIZE;

    vmm_map(pgd, virtio_mmio_virt_start, virtio_mmio_phys_start, PT_DEVICE_RW);
    vmm_flush_cache_range(virtio_mmio_virt_start, virtio_mmio_virt_end);

    // Map virtio PCIE mmio range
    size_t virtio_mmio_pci_size = 0x3f000000 - 0x10000000;
    uintptr_t virtio_mmio_pci_phys_start = 0x10000000;
    uintptr_t virtio_mmio_pci_virt_start = VMM_VIRT_BASE + virtio_mmio_pci_phys_start;
    uintptr_t virtio_mmio_pci_virt_end = virtio_mmio_pci_virt_start + virtio_mmio_pci_size;

    vmm_map_range(pgd, virtio_mmio_pci_virt_start, virtio_mmio_pci_virt_end, virtio_mmio_pci_phys_start, PT_DEVICE_RW);
    vmm_flush_cache_range(virtio_mmio_pci_virt_start, virtio_mmio_pci_virt_end);

    // Map GIC, UART, RTC, etc
    uintptr_t mmio_phys_start = 0x08000000;
    uintptr_t mmio_phys_end = 0x09030000;
    uintptr_t mmio_size = mmio_phys_end - mmio_phys_start;

    uintptr_t mmio_virt_start = (uintptr_t)VMM_VIRT_BASE + mmio_phys_start;
    uintptr_t mmio_virt_end = mmio_virt_start + mmio_size;

    vmm_map_range(pgd, mmio_virt_start, mmio_virt_end, mmio_phys_start, PT_DEVICE_RW);
    vmm_flush_cache_range(mmio_virt_start, mmio_virt_end);

    // Re-map kernel sections
    uintptr_t reqs_virt_start = (uintptr_t)__slimine_requests;
    uintptr_t reqs_virt_end = (uintptr_t)__elimine_requests;
    uintptr_t reqs_phys_start = (uintptr_t)kernel_phys + ((uintptr_t)reqs_virt_start - (uintptr_t)kernel_virt);

    vmm_map_range(pgd, reqs_virt_start, reqs_virt_end, reqs_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(reqs_virt_start, reqs_virt_end);

    uintptr_t text_virt_start = (uintptr_t)__stext;
    uintptr_t text_virt_end = (uintptr_t)__etext;
    uintptr_t text_phys_start = (uintptr_t)kernel_phys + ((uintptr_t)text_virt_start - (uintptr_t)kernel_virt);

    vmm_map_range(pgd, text_virt_start, text_virt_end, text_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(text_virt_start, text_virt_end);

    uintptr_t rodata_virt_start = (uintptr_t)__srodata;
    uintptr_t rodata_virt_end = (uintptr_t)__erodata;
    uintptr_t rodata_phys_start = (uintptr_t)kernel_phys + ((uintptr_t)rodata_virt_start - (uintptr_t)kernel_virt);

    vmm_map_range(pgd, rodata_virt_start, rodata_virt_end, rodata_phys_start, PT_KERNEL_RO);
    vmm_flush_cache_range(rodata_virt_start, rodata_virt_end);

    uintptr_t data_virt_start = (uintptr_t)__sdata;
    uintptr_t data_virt_end = (uintptr_t)__edata;
    uintptr_t data_phys_start = (uintptr_t)kernel_phys + (data_virt_start - kernel_virt);

    vmm_map_range(pgd, data_virt_start, data_virt_end, data_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(data_virt_start, data_virt_end);

    uintptr_t bss_virt_start = (uintptr_t)__sbss;
    uintptr_t bss_virt_end = (uintptr_t)__ebss;
    uintptr_t bss_phys_start = (uintptr_t)kernel_phys + (bss_virt_start - kernel_virt);

    vmm_map_range(pgd, bss_virt_start, bss_virt_end, bss_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(bss_virt_start, bss_virt_end);

    uintptr_t stack_virt_start = (uintptr_t)__sstack;
    uintptr_t stack_virt_end = (uintptr_t)__estack;    
    uintptr_t stack_phys_start = (uintptr_t)kernel_phys + (stack_virt_start - kernel_virt);

    vmm_map_range(pgd, stack_virt_start, stack_virt_end, stack_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(stack_virt_start, stack_virt_end);

    vmm_switch_pagemap(pgd);
    vmm_inval_all();

    printf("VMM: Initialized!\n");
}

void vmm_map_range(page_table_t *table, uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys, uint64_t flags) {
    assert(table != NULL);
    assert(virt_start <= virt_end);

    uintptr_t virt_start_aligned = ALIGN_DOWN(virt_start, PAGE_SIZE);
    uintptr_t virt_end_aligned = ALIGN_UP(virt_end, PAGE_SIZE);

    uintptr_t phys_aligned = ALIGN_DOWN(phys, PAGE_SIZE);

    size_t virt_size = virt_end_aligned - virt_start_aligned;
    size_t num_pages = SIZE_TO_PAGES(virt_size, PAGE_SIZE);

    for (size_t i = 0; i < num_pages; i++) {
        vmm_map(table, virt_start_aligned + (i * PAGE_SIZE), phys_aligned + (i * PAGE_SIZE), flags);
    }
}

void vmm_map(page_table_t *table, uintptr_t virt, uintptr_t phys, uint64_t flags) {
    uintptr_t l0_idx = PGD_IDX(virt);
    uintptr_t l1_idx = PUD_IDX(virt);
    uintptr_t l2_idx = PMD_IDX(virt);
    uintptr_t l3_idx = PTE_IDX(virt);




    assert(table != NULL);
    page_table_t *l0 = (page_table_t *)table;

    spinlock_acquire(&s);

    if (!(l0->entries[l0_idx] & PT_VALID)) {
        page_table_t *l1 = pmm_alloc();
        assert(l1 != NULL);

        l0->entries[l0_idx] = VIRT_TO_PHYS((uintptr_t)l0) | PT_TABLE | PT_VALID;
    }

    page_table_t *l1 = (page_table_t *)PHYS_TO_VIRT(l0->entries[l0_idx] & ~0xFFF);
    
    if (!(l1->entries[l1_idx] & PT_VALID)) {
        page_table_t *l2 = (page_table_t *)pmm_alloc();
        assert(l2 != NULL);

        l1->entries[l1_idx] = VIRT_TO_PHYS((uintptr_t)l2) | PT_TABLE | PT_VALID;
    }

    page_table_t *l2 = (page_table_t *)PHYS_TO_VIRT(l1->entries[l1_idx] & ~0xFFF);
    
    if (!(l2->entries[l2_idx] & PT_VALID)) {
        page_table_t *l3 = (page_table_t *)pmm_alloc();
        assert(l3 != NULL);

        l2->entries[l2_idx] = VIRT_TO_PHYS((uintptr_t)l3) | PT_TABLE | PT_VALID;
    }

    page_table_t *l3 = (page_table_t *)PHYS_TO_VIRT(l2->entries[l2_idx] & ~0xFFF);

    if (!(l3->entries[l3_idx] & PT_VALID)) {
        l3->entries[l3_idx] = (uintptr_t)phys | flags;
    }

    spinlock_release(&s);
}

void vmm_unmap(page_table_t *table, uintptr_t virt, uintptr_t phys) {
    uintptr_t l0_idx = PGD_IDX(virt);
    uintptr_t l1_idx = PUD_IDX(virt);
    uintptr_t l2_idx = PMD_IDX(virt);
    uintptr_t l3_idx = PTE_IDX(virt);


    
    assert(table != NULL);
    page_table_t *l0 = (page_table_t *)table;
    page_table_t *l1 = NULL, *l2 = NULL, *l3 = NULL;

    spinlock_acquire(&s);

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

    uintptr_t pte = l3->entries[l3_idx] & PT_PADDR_MASK;
    assert(pte == phys);

    l3->entries[l3_idx] = 0;
    vmm_inval_page((uintptr_t)pte);

    spinlock_release(&s);
}

void vmm_unmap_range(page_table_t *table, uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys_start) {
    assert(table != NULL);
    assert(virt_start < virt_end);

    size_t virt_size = virt_end - virt_start;
    size_t num_pages = SIZE_TO_PAGES(virt_size, PAGE_SIZE);

    for (size_t i = 0; i < num_pages; i++) {
        vmm_unmap(table, virt_start + (i * PAGE_SIZE), phys_start + (i * PAGE_SIZE));
    }
}

void vmm_inval_all(void) {
    __tlb_inval_all();
}

void vmm_inval_page(uintptr_t addr) {
    __tlb_inval_page(addr);
}

void vmm_switch_pagemap(page_table_t *table) {
    __ttbr1_write((uintptr_t)table);
}

void vmm_flush_dcache_addr(uintptr_t addr) {
    __dcache_flush_addr((uintptr_t)addr);
}

void vmm_flush_icache_addr(uintptr_t addr) {
    __icache_flush_addr((uintptr_t)addr);
}

void vmm_flush_cache_range(uintptr_t virt_start, uintptr_t virt_end) {
    __flush_cache_range((uintptr_t)virt_start, (uintptr_t)virt_end);
}