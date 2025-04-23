#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm.h>
#include <fs/mbr.h>
#include <fs/gpt.h>
#include <devices/virtio/virtio.h>
#include <devices/virtio/virtio_blk.h>




static gpt_t *gpt = NULL;
static gpt_partition_t *gpt_partitions = NULL;









void gpt_init(void) {
    uint32_t ret = 0;
    mbr_partition_t *mbr_part = NULL;
    size_t gpt_partiiton_size = 0;



    if (!gpt) {
        gpt = kmalloc(sizeof(gpt_t));

        if (!gpt) {
            return;
        }
    }

    mbr_part = mbr_get_partition_by_index(0);

    if (!mbr_part) {
        printf("GPT: Failed to get MBR partition index 0!\n");
        return;
    }

    if (mbr_part->partition_type != 0xEE) {
        printf("GPT: Invalid MBR partition type: 0x%lx\n", mbr_part->partition_type);
        return;
    }

    ret = virtio_blk_read((uint8_t *)gpt, mbr_part->start_lba, sizeof(gpt_t));

    if (ret != 0) {
        printf("GPT: Failed to read GPT from disk!\n");
        return;
    }

    if (gpt->signature != GPT_SIGNATURE) {
        printf("GPT: Invalid GPT signature! (got: 0x%lx, expected: 0x%lx)", gpt->signature, GPT_SIGNATURE);
        
        kfree((gpt_t *)gpt, sizeof(gpt_t));
        return;
    }

    gpt_partiiton_size = gpt->num_partition_entries * gpt->partition_entry_size;

    gpt_partitions = kmalloc(gpt_partiiton_size);

    if (!gpt_partitions) {
        printf("GPT: Failed to allocate GPT partition entries! (out of memory)\n");
        
        kfree((gpt_t *)gpt, sizeof(gpt_t));
        return;
    }

    ret = virtio_blk_read((uint8_t *)gpt_partitions, gpt->gpt_partition_entry_lba, gpt_partiiton_size);

    if (ret != 0) {
        printf("GPT: Failed to read GPT partition entries!\n");

        kfree((gpt_partition_t *)gpt_partitions, gpt_partiiton_size);
        return;
    }
}

gpt_partition_t *gpt_partition_entry_for_each(uint64_t idx) {
    gpt_partition_t *p = NULL;

    if (idx > gpt->num_partition_entries) {
        printf("GPT: Invalid partition index %llu!\n", idx);
        return NULL;
    }

    p = (gpt_partition_t *)((uint8_t *)gpt_partitions + idx * gpt->partition_entry_size);

    if (!p) {
        return NULL;
    }

    return p;
}

uint64_t gpt_partition_get_index(const char *name) {
    gpt_partition_t *p = NULL;
    uint64_t idx = 0;
    uint64_t max_entries = gpt->num_partition_entries;
    guid_t unused_entry = GPT_UNUSED_ENTRY_GUID;
    char partition_name[36];



    for (uint64_t i = 0; i < max_entries; i++) {
        p = gpt_partition_entry_for_each(i);

        char16_to_ascii(partition_name, (uint_least16_t *)p->partition_name, 32);

        if (strcmp(partition_name, name) == 0) {
            return i;
        }
    }

    return idx;
}

uint64_t gpt_partition_get_offset(uint64_t partition_idx) {
    gpt_partition_t *p = NULL;
    
    p = gpt_partition_entry_for_each(partition_idx);

    if (!p) {
        return -1;
    }

    return p->start_lba;
}