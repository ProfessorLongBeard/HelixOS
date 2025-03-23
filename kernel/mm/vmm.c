#include <mm.h>
#include <arch.h>
#include <kstdio.h>
#include <kstring.h>
#include <kstdlib.h>
#include <limine.h>
#include <kernel.h>
#include <framebuffer.h>






__attribute__((used, section(".limine_requests")))
static volatile struct limine_kernel_address_request kern_addr_req = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0  
};



static uint64_t *kernel_page_table;




void vmm_init(void) {
    struct limine_kernel_address_response *kern_addr = kern_addr_req.response;
    uint64_t hhdm = mm_get_hhdm_offset();
    uint64_t kernel_phys = kern_addr->physical_base;
    uint64_t kernel_virt = (uint64_t)__kernel_start;
    uint64_t kernel_virt_end = (uint64_t)__kernel_end;






    if (!kernel_page_table) {
        kernel_page_table = pmm_alloc();
    }


    struct limine_memmap_entry *fb_entry = mm_get_entry_by_type(LIMINE_MEMMAP_FRAMEBUFFER);
    assert(fb_entry != NULL);

    uint64_t fb_phys_base = fb_entry->base;
    uint64_t fb_virt_base = hhdm + fb_entry->base;
    uint64_t fb_virt_end = fb_virt_base + fb_entry->length;

    vmm_map_range(kernel_page_table, fb_virt_base, fb_virt_end, fb_phys_base, PT_DEVICE_RW);
    vmm_flush_cache_range(fb_virt_base, fb_virt_end);

    struct limine_memmap_entry *usable_mem = mm_get_entry_by_type(LIMINE_MEMMAP_USABLE);
    assert(usable_mem != NULL);

    uint64_t usable_mem_phys_base = usable_mem->base;
    uint64_t usable_mem_virt_base = hhdm + usable_mem->base;
    uint64_t usable_mem_virt_end = usable_mem_virt_base + usable_mem->length;

    vmm_map_range(kernel_page_table, usable_mem_virt_base, usable_mem_virt_end, usable_mem_phys_base, PT_KERNEL_RW);
    vmm_flush_cache_range(usable_mem_virt_base, usable_mem_virt_end);

    vmm_unmap(kernel_page_table, usable_mem_virt_base + PAGE_SIZE, usable_mem_phys_base + PAGE_SIZE);

    uint64_t reqs_virt_start = (uint64_t)__slimine_requests;
    uint64_t reqs_virt_end = (uint64_t)__elimine_requests;
    uint64_t reqs_phys_start = (uint64_t)kernel_phys + ((uint64_t)reqs_virt_start - (uint64_t)kernel_virt);

    vmm_map_range(kernel_page_table, reqs_virt_start, reqs_virt_end, reqs_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(reqs_virt_start, reqs_virt_end);

    uint64_t text_virt_start = (uint64_t)__stext;
    uint64_t text_virt_end = (uint64_t)__etext;
    uint64_t text_phys_start = (uint64_t)kernel_phys + ((uint64_t)text_virt_start - (uint64_t)kernel_virt);

    vmm_map_range(kernel_page_table, text_virt_start, text_virt_end, text_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(text_virt_start, text_virt_end);

    uint64_t rodata_virt_start = (uint64_t)__srodata;
    uint64_t rodata_virt_end = (uint64_t)__erodata;
    uint64_t rodata_phys_start = (uint64_t)kernel_phys + ((uint64_t)rodata_virt_start - (uint64_t)kernel_virt);

    vmm_map_range(kernel_page_table, rodata_virt_start, rodata_virt_end, rodata_phys_start, PT_KERNEL_RO);
    vmm_flush_cache_range(rodata_virt_start, rodata_virt_end);

    uint64_t data_virt_start = (uint64_t)__sdata;
    uint64_t data_virt_end = (uint64_t)__edata;
    uint64_t data_phys_start = (uint64_t)kernel_phys + (data_virt_start - kernel_virt);

    vmm_map_range(kernel_page_table, data_virt_start, data_virt_end, data_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(data_virt_start, data_virt_end);

    uint64_t bss_virt_start = (uint64_t)__sbss;
    uint64_t bss_virt_end = (uint64_t)__ebss;
    uint64_t bss_phys_start = (uint64_t)kernel_phys + (bss_virt_start - kernel_virt);

    vmm_map_range(kernel_page_table, bss_virt_start, bss_virt_end, bss_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(bss_virt_start, bss_virt_end);

    uint64_t stack_virt_start = (uint64_t)__sstack;
    uint64_t stack_virt_end = (uint64_t)__estack;    
    uint64_t stack_phys_start = (uint64_t)kernel_phys + (stack_virt_start - kernel_virt);

    vmm_map_range(kernel_page_table, stack_virt_start, stack_virt_end, stack_phys_start, PT_KERNEL_RW);
    vmm_flush_cache_range(stack_virt_start, stack_virt_end);

    vmm_switch_pagemap(kernel_page_table);
    vmm_inval_all();

    printf("VMM: Initialized!\n");
}

void vmm_map_range(uint64_t *table, uint64_t virt_start, uint64_t virt_end, uint64_t phys, uint64_t flags) {
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

void vmm_unmap_range(uint64_t *table, uint64_t virt_start, uint64_t virt_end) {
    assert(table != NULL);
    assert(virt_start < virt_end);

    uint64_t virt_size = virt_end - virt_start;
    uint64_t num_pages = SIZE_TO_PAGES(virt_size, PAGE_SIZE);

    for (uint64_t i = 0; i < num_pages; i++) {
        vmm_unmap(table, virt_start + (i * PAGE_SIZE), virt_end + (i * PAGE_SIZE));
    }
}

void vmm_map(uint64_t *table, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t l0_idx = PGD_IDX(virt);
    uint64_t l1_idx = PUD_IDX(virt);
    uint64_t l2_idx = PMD_IDX(virt);
    uint64_t l3_idx = PTE_IDX(virt);
    uint64_t hhdm = mm_get_hhdm_offset();




    assert(table != NULL);
    uint64_t *l0 = (uint64_t *)table;




    if (!(l0[l0_idx] & PT_VALID)) {
        uint64_t *l1 = (uint64_t *)pmm_alloc();
        assert(l1 != NULL);

        uint64_t l1_phys = ((uint64_t)l1 - hhdm);

        l0[l0_idx] = (uint64_t)l1_phys | PT_TABLE | PT_VALID;
    }

    uint64_t *l1 = (uint64_t *)((l0[l0_idx] & ~0xFFF) + hhdm);

    if (!(l1[l1_idx] & PT_VALID)) {
        uint64_t *l2 = (uint64_t *)pmm_alloc();
        assert(l2 != NULL);

        uint64_t l2_phys = ((uint64_t)l2 - hhdm);

        l1[l1_idx] = (uint64_t)l2_phys | PT_TABLE | PT_VALID;
    }

    uint64_t *l2 = (uint64_t *)((l1[l1_idx] & ~0xFFF) + hhdm);

    if (!(l2[l2_idx] & PT_VALID)) {
        uint64_t *l3 = (uint64_t *)pmm_alloc();
        assert(l3 != NULL);

        uint64_t l3_phys = ((uint64_t)l3 - hhdm);

        l2[l2_idx] = (uint64_t)l3_phys | PT_TABLE | PT_VALID;
    }

    uint64_t *l3 = (uint64_t *)((l2[l2_idx] & ~0xFFF) + hhdm);

    if (!(l3[l3_idx] & PT_VALID)) {
        l3[l3_idx] = phys | flags;
    }
}

void vmm_unmap(uint64_t *table, uint64_t virt, uint64_t phys) {
    uint64_t l0_idx = PGD_IDX(virt);
    uint64_t l1_idx = PUD_IDX(virt);
    uint64_t l2_idx = PMD_IDX(virt);
    uint64_t l3_idx = PTE_IDX(virt);
    uint64_t hhdm = mm_get_hhdm_offset();


    
    assert(table != NULL);
    uint64_t *l0 = (uint64_t *)table;
    uint64_t *l1 = NULL, *l2 = NULL, *l3 = NULL;


    if (l0[l0_idx & PT_VALID]) {
        l1 = (uint64_t *)((l0[l0_idx] & ~0xFFF) + hhdm);
    }

    if (l1[l1_idx] & PT_VALID) {
        l2 = (uint64_t *)((l1[l1_idx] & ~0xFFF) + hhdm);
    }
    
    if (l2[l2_idx] & PT_VALID) {
        l3 = (uint64_t *)((l2[l2_idx] & ~0xFFF) + hhdm);
    }

    uint64_t pte = l3[l3_idx] & PT_PADDR_MASK;
    assert(pte == phys);

    l3[l3_idx] = 0;
    vmm_inval_page((uint64_t)pte);
}

void vmm_inval_all(void) {
    __tlb_inval_all();
}

void vmm_inval_page(uint64_t addr) {
    __tlb_inval_page(addr);
}

void vmm_switch_pagemap(uint64_t *page_map) {
    __ttbr1_write((uint64_t)page_map);
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