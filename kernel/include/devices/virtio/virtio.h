#ifndef VIRTIO_H
#define VIRTIO_H

#include <stdint.h>
#include <stddef.h>
#include <mm/vmm.h>



/*
 * 0-23 Fearture bits for the specific device type
 * 24-37 Feature bits reserved for extentions
 * 38+ Feature bits reeserved for future extentions
 */

/*
 * Descriptor table: Alignment: 16, size: 16 * <queue size>
 * Available ring: Alignment: 2, size: 6 + 2 * <queue size>
 * Used ring: Alignment: 4, size = 6 + 8 * <queue size>
 *
 * Queue size is always a power of 2, and is a max size of 32K (32768)
 */





#define VIRTIO_MMIO_BASE        (VMM_VIRT_BASE + 0x0A000000)
#define VIRTIO_PCIE_MMIO_BASE   (VMM_VIRT_BASE + 0x10000000)
#define VIRTIO_PCIE_PIO_BASE    (VMM_VIRT_BASE + 0x3EFF0000)
#define VIRTIO_PCIE_ECAM_BASE   (VMM_VIRT_BASE + 0x3F000000)

#define VIRTIO_IRQ_ID           48

#define VIRTIO_QUEUE_SIZE       128

#define VIRTIO_ALIGN(x, align) (((x) + (align) - 1) & ~((align) - 1))

#define VIRTIO_DESC_ALIGN(qsz)  VIRTIO_ALIGN((qsz), 16)
#define VIRTIO_AVAIL_ALIGN(qsz) VIRTIO_ALIGN((qsz), 2)
#define VIRTIO_USED_ALIGN(qsz)  VIRTIO_ALIGN((qsz), 4)

#define VIRTIO_MAGIC            0x74726976  // "virt" string in little endian
#define VIRTIO_VERSION_LEGACY   0x1         // v1 -> Legacy device
#define VIRTIO_VERSION          0x2         // v2 -> Modern device


#define VIRTIO_DEVICE_RESET         0x0     // Reset the device
#define VIRTIO_DEVICE_ACK           0x1     // Indicates guest OS has found the device, and is valid
#define VIRTIO_DEVICE_DRIVER        0x2     // Indicates guest OS knows how to drive the device
#define VIRTIO_DEVICE_FAILED        0x80    // Indicates that something has went wrong in the guest
#define VIRTIO_DEVICE_FEATURES_OK   0x8     // Indicates that the driver has acknowledged the features it understands
#define VIRTIO_DEVICE_DRIVER_OK     0x4     // Indicates that the driver is setup, and ready
#define VIRTIO_DEVICE_NEEDS_RESET   0x40    // Indicates that the device has experienced an error, and can't recover

#define VIRTIO_RING_EVENT_FLAGS_ENABLE      0x0
#define VIRTIO_RING_EVENT_FLAGS_DISABLE     0x1
#define VIRTIO_RING_EVENT_FLAGS_DESC        0x2

#define VIRTIO_BLK_F_SIZE_MAX           1
#define VIRTIO_BLK_F_SEG_MAX            2
#define VIRTIO_BLK_F_GEOMETRY           4
#define VIRTIO_BLK_F_RO                 5
#define VIRTIO_BLK_F_BLK_SIZE           6
#define VIRTIO_BLK_F_FLUSH              9
#define VIRTIO_BLK_F_TOPOLOGY           10
#define VIRTIO_BLK_F_CONFIG_WCE         11
#define VIRTIO_BLK_F_DISCARD            13
#define VIRTIO_BLK_F_WRITE_ZEROS        14
#define VIRTIO_BLK_F_BARRIER            0
#define VIRTIO_BLK_F_SCSI               7
#define VIRTIO_BLK_F_NOTIFY_ON_EMPTY    24
#define VIRTIO_BLK_F_ANY_LAYOUT         27
#define VIRTIO_BLK_F_RING_INDIRECT_DESC 28
#define VIRTIO_BLK_F_RING_EVENT_IDX     29
#define VIRTIO_BLK_F_VERSION_1          32

#define VIRTIO_AVAIL_F_NO_INTERRUPTS    1

#define VIRTIO_BLK_T_IN             0
#define VIRTIO_BLK_T_OUT            1
#define VIRTIO_BLK_T_FLUSH          4
#define VIRTIO_BLK_T_DISCARD        11
#define VIRTIO_BLK_T_WRITE_ZEROS    13

#define VIRTIO_BLK_S_OK     0
#define VIRTIO_BLK_S_IOERR  1
#define VIRTIO_BLK_S_UNSUPP 2

#define VIRTIO_DESC_F_NEXT      1   // Marks buffer as containing next descriptor field
#define VIRTIO_DESC_F_WRITE     2   // Makrs the buffer as write-only (otherwise read-only)
#define VIRTIO_DESC_F_INDIRECT  4   // Means buffer contains a list of buffer descriptors

#define RING_EVENT_FLAGS_ENABLE     0x0     // Enable events
#define RING_EVENT_FLAGS_DISABLE    0x1     // Disable events
#define RING_EVENT_FLAGS_DESC       0x2     // Enable events for specific descriptor



// TODO:
// - Ensure structs that need to have volatile are set to avoid issues when removing -O0




typedef struct {
    volatile uint64_t    addr;
    volatile uint32_t    length;
    volatile uint16_t    flags;
    volatile uint16_t    next;
} __attribute__((packed)) virtio_desc_t;

typedef struct {
    volatile uint16_t       flags;
    volatile uint16_t       index;
    volatile uint16_t       ring[VIRTIO_QUEUE_SIZE];
    volatile uint16_t       event;
} __attribute__((packed)) virtio_avail_t;

typedef struct {
    uint32_t    id;
    uint32_t    length;
} __attribute__((packed)) virtio_used_elem_t;

typedef struct {
    volatile uint16_t           flags;
    volatile uint16_t           index;
    volatile virtio_used_elem_t ring[VIRTIO_QUEUE_SIZE];
    volatile uint16_t           event;
} __attribute__((packed)) virtio_used_t;

typedef struct virtio_queue_t {
    volatile uint16_t       free_desc;
    volatile uint16_t       last_used;
    volatile virtio_desc_t  desc[VIRTIO_QUEUE_SIZE];
    volatile virtio_avail_t avail;
    volatile uint8_t        pad[VIRTIO_ALIGN(sizeof(virtio_avail_t), PAGE_SIZE)];
    volatile virtio_used_t  used;
} __attribute__((packed)) virtio_queue_t;

typedef struct {
    uint32_t    magic;
    uint32_t    version;
    uint32_t    device_id;
    uint32_t    vendor_id;
    uint32_t    device_features;
    uint32_t    device_features_sel;
    uint32_t    __pad0[2];
    uint32_t    driver_features;
    uint32_t    driver_features_sel;
    uint32_t    __pad1[2];
    uint32_t    queue_sel;
    uint32_t    queue_num_max;
    uint32_t    queue_num;
    uint32_t    __pad3[2];
    uint32_t    queue_ready;
    uint32_t    __pad4[2];
    uint32_t    queue_notify;
    uint32_t    __pad5[3];
    uint32_t    interrupt_status;
    uint32_t    interrupt_ack;
    uint32_t    __pad6[2];
    uint32_t    status;
    uint32_t    __pad7[3];
    uint32_t    queue_desc_low;
    uint32_t    queue_desc_high;
    uint32_t    __pad8[2];
    uint32_t    queue_avail_low;
    uint32_t    queue_avail_high;
    uint32_t    __pad9[2];
    uint32_t    queue_used_low;
    uint32_t    queue_used_high;
    uint32_t    __pad10[21];
    uint32_t    config_generations;
    uint32_t    config;
} __attribute__((packed)) virtio_t;

typedef struct {
    uint16_t    cylinders;
    uint8_t     heads;
    uint8_t     tracks;
} virtio_blk_geometry_t;

typedef struct {
    uint8_t     phys_block_exp;
    uint8_t     alignement_offset;
    uint16_t    min_io_size;
    uint32_t    opt_io_size;
} virtio_blk_topology_t;

typedef struct {
    uint64_t                capacity;
    uint32_t                size_max;
    uint32_t                seg_max;
    virtio_blk_geometry_t   geometry;
    uint32_t                block_size;
    virtio_blk_topology_t   topology;
    uint8_t                 writeback;
    uint8_t                 __unused0[3];
    uint32_t                max_discard_sectors;
    uint32_t                max_discard_seg;
    uint32_t                discard_sector_alignment;
    uint32_t                max_write_zeros_sectors;
    uint32_t                max_write_zeros_seg;
    uint8_t                 write_zeros_may_unmap;
    uint8_t                 __unused1[3];
} __attribute__((packed)) virtio_blk_config_t;

typedef struct {
    uint32_t    type;
    uint32_t    reserved;
    uint64_t    sector;
    uint8_t     data[512];
    uint8_t     status;
} __attribute__((packed)) virtio_blk_req_t;

extern virtio_queue_t *vq;








void virtio_init(void);
void virtio_irq_handler(void);
void virtio_submit_req(uint16_t desc_index);
uint32_t virtio_get_block_size(void);

#endif