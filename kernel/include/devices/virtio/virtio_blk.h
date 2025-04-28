#ifndef VIRTIO_BLK_H
#define VIRTIO_BLK_H

#include <stdint.h>
#include <stddef.h>
#include <devices/virtio/virtio.h>



void virtio_blk_init(void);
size_t virtio_pad_bytes(size_t length);
virtio_desc_t *virtio_desc_alloc(uint32_t *idx);
void virtio_desc_free(uint16_t index);
void virtio_desc_free_multiple(uint16_t desc_index);
uint32_t virtio_blk_read(uint8_t *buf, uint64_t sector, size_t length);
uint32_t virtio_blk_write(uint8_t *buf, uint64_t sector, size_t length);
virtio_queue_t *virtio_blk_queue_init(size_t queue_size);

#endif