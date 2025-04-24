#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <limine.h>
#include <spinlock.h>






#define VMM_VIRT_BASE   (0xFFFF000000000000ULL)

#define PT_VIRT_BASE    (0xFFFF800000000000ULL)
#define PT_VIRT_END     (0xFFFF800004000000ULL)



#define VIRT_TO_PHYS(virt) ((virt) - VMM_VIRT_BASE)
#define PHYS_TO_VIRT(phys) ((phys) + VMM_VIRT_BASE)

#define PT_ENTRY_SIZE   512
#define PT_POOL_SIZE    (64 * 1024 * 1024)

#define PAGE_SIZE       4096

#define ALIGN_UP(base, align) ((base + (align - 1)) & ~(align - 1))
#define ALIGN_DOWN(base, align) (base & ~(align - 1))
#define DIV_ROUNDUP(x, y) (((x) + (y) - 1) / (y))

#define SIZE_TO_PAGES(size, page_size) ((size + (page_size - 1)) / page_size)

#define PAGE_IS_ALIGNED(addr, size) (((addr) & ((size) - 1)) == 0)

#define PAGE_MASK       (0xFFFFFFFFF000ULL)
#define PT_PADDR_MASK   (0x0000FFFFFFFFF000ULL)

#define PT_PAGE     (1UL << 1UL)
#define PT_BLOCK    (0UL << 1UL)
#define PT_TABLE    (1UL << 1UL)

#define PT_VALID    (1UL << 0UL)

#define PT_NS       (1UL << 5UL)

#define PT_RW_EL1   (0UL << 6UL)
#define PT_RO_EL1   (1UL << 6UL)
#define PT_RW_EL0   (1UL << 6UL)
#define PT_RO_EL0   (2UL << 6UL)

#define PT_SH_NONE      (0UL << 8UL)
#define PT_SH_OUTER     (1UL << 8UL)
#define PT_SH_INNER     (2UL << 8UL)

#define PT_AF       (1UL << 10UL) // Access flag
#define PT_NG       (1UL << 11UL) // Non-global
#define PT_DBM      (1UL << 51UL) // Dirty bit modifier
#define PT_CONTIG   (1UL << 52UL) // Continguous
#define PT_PXN      (1UL << 53UL) // Privilages execute never
#define PT_UXN      (1UL << 54UL) // User execute never

#define PT_KERNEL   (1UL << 63UL)

#define PT_ATTRIDX(x) ((x) << 2)

#define PT_NORMAL   PT_ATTRIDX(0)
#define PT_DEVICE   PT_ATTRIDX(1)

#define PT_USER_RW      (PT_PAGE | PT_VALID | PT_AF | PT_NORMAL | PT_RW_EL0 | PT_SH_INNER)
#define PT_USER_RO      (PT_PAGE | PT_VALID | PT_AF | PT_NORMAL | PT_RO_EL0 | PT_SH_INNER)

// Read-only reserved user page attributes
#define PT_USER_NA      (PT_PAGE | PT_VALID | PT_NORMAL | PT_RO_EL0 | PT_SH_INNER)

#define PT_KERNEL_RW    (PT_PAGE | PT_VALID | PT_AF | PT_NORMAL | PT_RW_EL1 | PT_SH_INNER | PT_KERNEL)
#define PT_KERNEL_RO    (PT_PAGE | PT_VALID | PT_AF | PT_NORMAL | PT_RO_EL1 | PT_SH_INNER | PT_KERNEL)

// Read-only reserved kernel page attributes
#define PT_KERNEL_NA    (PT_PAGE | PT_VALID | PT_NORMAL | PT_RO_EL1 | PT_SH_INNER | PT_KERNEL)

#define PT_DEVICE_RW    (PT_PAGE | PT_VALID | PT_AF | PT_DEVICE | PT_RW_EL1 | PT_SH_NONE | PT_KERNEL)
#define PT_DEVICE_RO    (PT_PAGE | PT_VALID | PT_AF | PT_DEVICE | PT_RO_EL1 | PT_SH_NONE | PT_KERNEL)

#define L0_SHIFT    39UL
#define L1_SHIFT    30UL
#define L2_SHIFT    21UL
#define L3_SHIFT    12UL

#define L0_BLOCK_SIZE   (1UL << L0_SHIFT)
#define L1_BLOCK_SIZE   (1UL << L1_SHIFT)
#define L2_BLOCK_SIZE   (1UL << L2_SHIFT)
#define L3_BLOCK_SIZE   (1UL << L3_SHIFT)

#define PGD_IDX(virt) (((virt) >> L0_SHIFT) & 0x1FF)
#define PUD_IDX(virt) (((virt) >> L1_SHIFT) & 0x1FF)
#define PMD_IDX(virt) (((virt) >> L2_SHIFT) & 0x1FF)
#define PTE_IDX(virt) (((virt) >> L3_SHIFT) & 0x1FF)

typedef struct {
    uintptr_t entries[PT_ENTRY_SIZE];
} __attribute__((aligned(PAGE_SIZE), packed)) page_table_t;













void vmm_init(uintptr_t kernel_phys, uintptr_t kernel_virt, struct limine_memmap_response *m);
void vmm_inval_page(uintptr_t addr);
void vmm_inval_all(void);
void vmm_switch_pagemap(page_table_t *table);
void vmm_flush_dcache_addr(uintptr_t addr);
void vmm_flush_icache_addr(uintptr_t addr);
void vmm_flush_cache_range(uintptr_t virt_start, uintptr_t virt_end);
page_table_t *vmm_pt_alloc(void);
void vmm_pt_dealloc(page_table_t *table);
void vmm_map(page_table_t *table, uintptr_t virt, uintptr_t phys, uint64_t flags);
void vmm_map_range(page_table_t *table, uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys, uint64_t flags);
void vmm_unmap(page_table_t *table, uintptr_t virt, uintptr_t phys);
void vmm_unmap_range(page_table_t *table, uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys_start);
page_table_t *vmm_get_pgd(void);

#endif