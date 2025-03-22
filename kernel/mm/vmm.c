#include <mm.h>
#include <arch.h>
#include <kstdio.h>
#include <kstring.h>
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
    uint64_t kernel_virt = kern_addr->virtual_base;
    uint64_t kernel_phys = kern_addr->physical_base;
    kernel_page_table = pmm_alloc();
    uint64_t hhdm = mm_get_hhdm_offset();
    uint64_t bitmap_base = pmm_get_bitmap_base();
    size_t bitmap_size = pmm_get_bitmap_size();
    uint64_t bitmap_end = bitmap_base + bitmap_size;
    uint64_t fb_base = fb_get_base();
    uint64_t fb_size = fb_get_size();
    uint64_t fb_end = fb_base + fb_size; 






    for (uint64_t bmp = ALIGN_DOWN(bitmap_base, PAGE_SIZE); bmp < ALIGN_UP(bitmap_end, PAGE_SIZE); bmp += PAGE_SIZE) {
        vmm_map(kernel_page_table, bmp, bmp - hhdm, PT_PAGE | PT_AF | PT_VALID | PT_RW | PT_SH_IS | PT_KERNEL);
    }

    for (uint64_t fb = ALIGN_DOWN(fb_base, PAGE_SIZE); fb < ALIGN_UP(fb_end, PAGE_SIZE); fb += PAGE_SIZE) {
        vmm_map(kernel_page_table, fb, fb - hhdm, PT_PAGE | PT_AF | PT_VALID | PT_RW | PT_KERNEL);
    }

    uint64_t reqs_virt_start = (uint64_t)__slimine_requests;
    uint64_t reqs_virt_end = (uint64_t)__elimine_requests;
    size_t reqs_size = reqs_virt_end - reqs_virt_start;

    uint64_t reqs_phys_start = (uint64_t)kernel_phys + ((uint64_t)reqs_virt_start - (uint64_t)kernel_virt);
    uint64_t reqs_phys_aligned = ALIGN_DOWN(reqs_phys_start, PAGE_SIZE);

    for (uint64_t req = ALIGN_DOWN((uint64_t)reqs_virt_start, PAGE_SIZE); req < ALIGN_UP(reqs_virt_end, PAGE_SIZE); req += PAGE_SIZE) {
        vmm_map(kernel_page_table, req, reqs_phys_aligned, PT_PAGE | PT_AF | PT_VALID | PT_RW | PT_SH_IS | PT_KERNEL);
        reqs_phys_aligned += PAGE_SIZE;
    }

    uint64_t text_virt_start = (uint64_t)__stext;
    uint64_t text_virt_end = (uint64_t)__etext;

    uint64_t text_phys_start = (uint64_t)kernel_phys + ((uint64_t)text_virt_start - (uint64_t)kernel_virt);
    uint64_t text_phys_aligned = ALIGN_DOWN(text_phys_start, PAGE_SIZE);

    for (uint64_t text = ALIGN_DOWN(text_virt_start, PAGE_SIZE); text < ALIGN_UP(text_virt_end, PAGE_SIZE); text += PAGE_SIZE) {
        vmm_map(kernel_page_table, text, text_phys_aligned, PT_PAGE | PT_AF | PT_VALID | PT_SH_IS | PT_KERNEL);
        text_phys_aligned += PAGE_SIZE;
    }
    
    uint64_t rodata_virt_start = (uint64_t)__srodata;
    uint64_t rodata_virt_end = (uint64_t)__erodata;

    uint64_t rodata_phys_start = (uint64_t)kernel_phys + ((uint64_t)rodata_virt_start - (uint64_t)kernel_virt);
    uint64_t rodata_phys_aligned = ALIGN_DOWN(rodata_phys_start, PAGE_SIZE);

    for (uint64_t rodata = ALIGN_DOWN(rodata_virt_start, PAGE_SIZE); rodata < ALIGN_UP(rodata_virt_end, PAGE_SIZE); rodata += PAGE_SIZE) {
        vmm_map(kernel_page_table, rodata, rodata_phys_aligned, PT_PAGE | PT_AF | PT_VALID | PT_SH_IS | PT_KERNEL);
        rodata_phys_aligned += PAGE_SIZE;
    }

    uint64_t data_virt_start = (uint64_t)__sdata;
    uint64_t data_virt_end = (uint64_t)__edata;

    uint64_t data_phys_start = (uint64_t)kernel_phys + (data_virt_start - kernel_virt);
    uint64_t data_phys_aligned = ALIGN_DOWN(data_phys_start, PAGE_SIZE);

    for (uint64_t data = ALIGN_DOWN(data_virt_start, PAGE_SIZE); data < ALIGN_UP(data_virt_end, PAGE_SIZE); data += PAGE_SIZE) {
        vmm_map(kernel_page_table, data, data_phys_aligned, PT_PAGE | PT_AF | PT_VALID | PT_RW | PT_SH_IS | PT_KERNEL);
        data_phys_aligned += PAGE_SIZE;
    }

    uint64_t bss_virt_start = (uint64_t)__sbss;
    uint64_t bss_virt_end = (uint64_t)__ebss;

    uint64_t bss_phys_start = (uint64_t)kernel_phys + (bss_virt_start - kernel_virt);
    uint64_t bss_phys_aligned = ALIGN_DOWN(bss_phys_start, PAGE_SIZE);
    
    for (uint64_t bss = ALIGN_DOWN(bss_virt_start, PAGE_SIZE); bss < ALIGN_UP(bss_virt_end, PAGE_SIZE); bss += PAGE_SIZE) {
        vmm_map(kernel_page_table, bss, bss_phys_aligned, PT_PAGE | PT_AF | PT_VALID | PT_RW | PT_SH_IS | PT_KERNEL);
        bss_phys_aligned += PAGE_SIZE;
    }

    uint64_t stack_virt_start = (uint64_t)__sstack;
    uint64_t stack_virt_end = (uint64_t)__estack;
    
    uint64_t stack_phys_start = (uint64_t)kernel_phys + (stack_virt_start - kernel_virt);
    uint64_t stack_phys_aligned = ALIGN_DOWN(stack_phys_start, PAGE_SIZE);

    for (uint64_t stack = ALIGN_DOWN(stack_virt_start, PAGE_SIZE); stack < ALIGN_UP(stack_virt_end, PAGE_SIZE); stack += PAGE_SIZE) {
        vmm_map(kernel_page_table, stack, stack_phys_aligned, PT_PAGE | PT_AF | PT_VALID | PT_RW | PT_SH_IS | PT_KERNEL);
        stack_phys_aligned += PAGE_SIZE;
    }

    vmm_switch_pagemap(kernel_page_table);

    printf("VMM: Done!\n");
}

void vmm_map(uint64_t *table, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t l0_idx = (virt >> L0_SHIFT) & 0x1FF;
    uint64_t l1_idx = (virt >> L1_SHIFT) & 0x1FF;
    uint64_t l2_idx = (virt >> L2_SHIFT) & 0x1FF;
    uint64_t l3_idx = (virt >> L3_SHIFT) & 0x1FF;
    uint64_t hhdm = mm_get_hhdm_offset();




    uint64_t *l0 = (uint64_t *)table;


    if (!(l0[l0_idx] & PT_VALID)) {
        uint64_t *l1 = (uint64_t *)pmm_alloc();
        uint64_t l1_phys = ((uint64_t)l1 - hhdm);

        l0[l0_idx] = (uint64_t)l1_phys | PT_TABLE | PT_VALID;
    }

    uint64_t *l1 = (uint64_t *)((l0[l0_idx] & ~0xFFF) + hhdm);

    if (!(l1[l1_idx] & PT_VALID)) {
        uint64_t *l2 = (uint64_t *)pmm_alloc();
        uint64_t l2_phys = ((uint64_t)l2 - hhdm);

        l1[l1_idx] = (uint64_t)l2_phys | PT_TABLE | PT_VALID;
    }

    uint64_t *l2 = (uint64_t *)((l1[l1_idx] & ~0xFFF) + hhdm);

    if (!(l2[l2_idx] & PT_VALID)) {
        uint64_t *l3 = (uint64_t *)pmm_alloc();
        uint64_t l3_phys = ((uint64_t)l3 - hhdm);

        l2[l2_idx] = (uint64_t)l3_phys | PT_TABLE | PT_VALID;
    }

    uint64_t *l3 = (uint64_t *)((l2[l2_idx] & ~0xFFF) + hhdm);

    if (!(l3[l3_idx] & PT_VALID)) {
        l3[l3_idx] = phys | flags;
    }
}

void vmm_inval_page(uint64_t addr) {
    __tlb_inval_page(addr);
}

void vmm_switch_pagemap(uint64_t *page_map) {
    __ttbr1_write((uint64_t)page_map);
}