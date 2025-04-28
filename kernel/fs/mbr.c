#include <kstdio.h>
#include <kstdlib.h>
#include <mm/mm.h>
#include <fs/mbr.h>
#include <devices/virtio/virtio.h>
#include <devices/virtio/virtio_blk.h>





static mbr_t *mbr = NULL;





void mbr_init(void) {
    uint32_t ret = 0;

    if (!mbr) {
        mbr = kmalloc(sizeof(mbr_t));
    }

    ret = virtio_blk_read((uint8_t *)mbr, 0, sizeof(mbr_t));

    if (ret != 0) {
        printf("MBR: Failed to read MBR from device!\n");
        return;
    }
}

mbr_partition_t *mbr_get_partition_by_index(int idx) {
    mbr_partition_t *part = NULL;

    if (!mbr) {
        return NULL;
    }

    if (idx < 0 || idx > 4) {
        printf("MBR: Invalid MBR partition index %d\n", idx);
        return NULL;
    }

    part = (mbr_partition_t *)&mbr->partitions[idx];

    return part;
}