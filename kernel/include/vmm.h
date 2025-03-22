#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <pmm.h>





#define ENTRY_SIZE          512

#define PAGE_IS_ALIGNED(addr, size) (((addr) & ((size) - 1)) == 0)

#define PAGE_MASK   0xFFFFFFFFF000
#define PT_PADDR_MASK    ((uint64_t)0x0000FFFFFFFFF000)

#define PT_PAGE     (1UL << 1UL)
#define PT_BLOCK    (0UL << 1UL)

#define PT_TABLE    (1UL << 1UL)


#define PT_VALID    (1UL << 0UL)

#define PT_RW       (1UL << 2UL)

#define PT_NS       (1UL << 5UL)
#define PT_USER     (1UL << 6UL)

#define PT_SH_NS    (0UL << 8UL)
#define PT_SH_OS    (1UL << 8UL)
#define PT_SH_IS    (2UL << 8UL)

#define PT_AF       (1UL << 10UL) // Access flag
#define PT_NG       (1UL << 11UL) // Non-global
#define PT_DBM      (1UL << 51UL) // Dirty bit modifier
#define PT_CONTIG   (1UL << 52UL) // Continguous
#define PT_PXN      (1UL << 53UL) // Privilages execute never
#define PT_UXN      (1UL << 54UL) // User execute never

#define PT_KERNEL   (1UL << 63UL)

#define L0_SHIFT    39UL
#define L1_SHIFT    30UL
#define L2_SHIFT    21UL
#define L3_SHIFT    12UL

#define L0_BLOCK_SIZE   (1UL << L0_SHIFT)
#define L1_BLOCK_SIZE   (1UL << L1_SHIFT)
#define L2_BLOCK_SIZE   (1UL << L2_SHIFT)
#define L3_BLOCK_SIZE   (1UL << L3_SHIFT)


typedef struct {
    uint64_t entries[ENTRY_SIZE];
} page_table_t;






void vmm_init(void);
void vmm_inval_page(uint64_t addr);
void vmm_switch_pagemap(uint64_t *page_map);
void vmm_map(uint64_t *table, uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_map_kernel_section(uint64_t *table, uint64_t virt_start, uint64_t virt_end, uint64_t flags);

#endif