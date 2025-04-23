#ifndef GPT_H
#define GPT_H

#include <stdint.h>
#include <stddef.h>





#define GPT_SIGNATURE 0x5452415020494645ULL

#define GPT_UNUSED_ENTRY_GUID \
    {0x00000000, 0x0000, 0x0000, 0x00, 0x00, \
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}




typedef struct {
    uint32_t    time_low;
    uint16_t    time_mid;
    uint16_t    time_high_and_ver;
    uint8_t     clock_seq_high;
    uint8_t     clock_seq_low;
    uint8_t     node[6];
} __attribute__((packed)) guid_t;

typedef struct {
    uint64_t    signature;
    uint32_t    gpt_revision;
    uint32_t    header_size;
    uint32_t    crc32_checksum;
    uint32_t    __reserved;
    uint64_t    gpt_header_lba;
    uint64_t    alt_gpt_header_lba;
    uint64_t    first_gpt_entry_lba;
    uint64_t    last_gpt_entry_lba;
    guid_t      gpt_guid;
    uint64_t    gpt_partition_entry_lba;
    uint32_t    num_partition_entries;
    uint32_t    partition_entry_size;
    uint32_t    partition_entry_crc32;
} __attribute__((packed)) gpt_t;

typedef struct {
    guid_t      partition_type_guid;
    guid_t      unique_partition_guid;
    uint64_t    start_lba;
    uint64_t    end_lba;
    uint64_t    attributes;
    char        partition_name[36];
} __attribute__((packed)) gpt_partition_t;





uint64_t gpt_partition_get_index(const char *name);
gpt_partition_t *gpt_partition_entry_for_each(uint64_t idx);
uint64_t gpt_partition_get_index(const char *name);
uint64_t gpt_partition_get_offset(uint64_t partition_idx);

#endif