#ifndef MBR_H
#define MBR_H

#include <stdint.h>
#include <stddef.h>


#define MBR_BOOTCODE_LEN            440

#define MBR_BOOT_SIGNATURE          0xAA55
#define MBR_PROTECTIVE_SIGNATURE    0xEE




typedef struct {
    uint8_t     drive_attributes;
    uint8_t     start_chs[3];
    uint8_t     partition_type;
    uint8_t     end_chd[3];
    uint32_t    start_lba;
    uint32_t    num_sectors;
} __attribute__((packed)) mbr_partition_t;

typedef struct {
    uint8_t         bootcode[MBR_BOOTCODE_LEN];
    uint32_t        disk_signature;
    uint16_t        reserved;
    mbr_partition_t partitions[4];
    uint16_t        boot_signature;
} __attribute__((packed)) mbr_t;







void mbr_init(void);
mbr_partition_t *mbr_get_partition_by_index(int idx);

#endif