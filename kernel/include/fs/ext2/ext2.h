#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>
#include <stddef.h>
#include <fs/vfs.h>
#include <spinlock.h>
#include <fs/ext2/ext2_utils.h>






#define EXT2_SIGNATURE  0xEF53

#define EXT2_MIN_BLOCK_SIZE     1024
#define	EXT2_MAX_BLOCK_SIZE	    65536

#define EXT2_TYPE_MASK  0xF000
#define EXT2_PERM_MASK  0x0FFF

#define EXT2_BAD_BLOCKS_INODE       1
#define EXT2_ROOT_INODE             2
#define EXT2_BOOT_LOADER_INODE      5
#define EXT2_UNDELETE_DIR_INODE     6

#define EXT2_SUPERBLOCK_SIZE    1024

#define EXT2_STATE_CLEAN  1
#define EXT2_STATE_ERROR  2

#define EXT2_ERROR_STATE_IGNORE       1   // Ignore errors, and continue
#define EXT2_ERROR_STATE_REMOUNT_RO   2   // Remount filesystem as read-only
#define EXT2_ERROR_STATE_KERNEL_PANIC 3   // Kernel panic

#define EXT2_OS_ID_LINIUX       0
#define EXT2_OS_ID_GNU_HURD     1
#define EXT2_OS_ID_MASIX        2
#define EXT2_OS_ID_FREEBSD      3
#define EXT2_OS_ID_OTHER        4

// Optional features
#define EXT2_PREALLOC_BLOCKS                    0x0001
#define EXT2_ASF_SERVER_INODE_EXISTS            0x0002
#define EXT2_HAS_JOURNAL                        0x0004
#define EXT2_INODES_HAVE_EXT_ATTRIBUTES         0x0008
#define EXT2_CAN_RESIZE_FOR_LARGER_PARTITIONS   0x0010
#define EXT2_DIRECTORIES_USE_HASH_INDEX         0x0020

// Required features
#define EXT2_COMPRESISON_USED                   0x0001
#define EXT2_DIRS_ENTIRES_CONTAIN_TYPE_LIST     0x0002
#define EXT2_JOURNAL_NEEDS_REPLAY               0x0004
#define EXT2_JOURNAL_USES_DEVICE                0x0008

// Read-only features
#define EXT2_SPARSE_SUPERBLOCK                  0x0001
#define EXT2_USES_64BIT_FILE_SIZE               0x0002
#define EXT2_DIRS_STORED_IN_BIN_TREE            0x0004

// EXT2 file types
#define EXT2_TYPE_FIFO          0x1000
#define EXT2_TYPE_CHAR_DEV      0x2000
#define EXT2_TYPE_DIRECTORY     0x4000
#define EXT2_TYPE_BLOCK_DEV     0x6000
#define EXT2_TYPE_REG_FILE      0x8000
#define EXT2_TYPE_SYMLINK       0xA000
#define EXT2_TYPE_SOCKET        0xC000

// EXT2 permission types
#define EXT2_S_SETUID           04000
#define EXT2_S_SETGID           02000
#define EXT2_S_STICKY           01000
#define EXT2_S_IRUSR            00400
#define EXT2_S_IWUSR            00200
#define EXT2_S_IXUSR            00100
#define EXT2_S_IRGRP            00040
#define EXT2_S_IWGRP            00020
#define EXT2_S_IXGRP            00010
#define EXT2_S_IROTH            00004
#define EXT2_S_IWOTH            00002
#define EXT2_S_IXOTH            00001

// inode flags
#define EXT2_INODE_SECURE_DELETE    0x00000001
#define EXT2_INODE_KEEP_COPY        0x00000002
#define EXT2_INODE_FILE_COMPRESS    0x00000004
#define EXT2_SYNCHRONOUS_UPDATES    0x00000008
#define EXT2_IMMUTABLE_FILE         0x00000010
#define EXT2_APPEND_ONLY            0x00000020
#define EXT2_FILE_NOT_INCLUDE       0x00000040
#define EXT2_LAST_ACCESS_NO_UPDATE  0x00000080
#define EXT2_HASHED_INDEX_DIRECTORY 0x00010000
#define EXT2_AFS_DIRECTORY          0x00020000
#define EXT2_JOURNAL_DATA_FILE      0x00040000

// Directory entry type
#define EXT2_DIR_TYPE_UNKNOWN       0
#define EXT2_DIR_TYPE_REG_FILE      1
#define EXT2_DIR_TYPE_DIR           2
#define EXT2_DIR_TYPE_CHAR_DEV      3
#define EXT2_DIR_TYPE_BLOCK_DEV     4
#define EXT2_DIR_TYPE_FIFO          5
#define EXT2_DIR_TYPE_SOCKET        6
#define EXT2_DIR_TYPE_SYMLINK       7

#define EXT2_GET_INODE_DATA_SIZE(inode) (((size_t)(inode)->hi_size_bytes << 32) | (inode)->lo_size_bytes)

#define EXT2_LBA_TO_OFFSET(lba, block_size) ((lba) * (block_size))
#define EXT2_OFFSET_TO_LBA(lba, block_size) ((lba) / (block_size))

#define EXT2_GET_INODE_SIZE(sb) ((sb)->inode_structure_size)

#define EXT2_GET_BLOCK_SIZE(sb) (1024 << (sb)->log2_block_size)
#define EXT2_GET_FRAG_SIZE(sb)  (1024 << (sb)->log2_fragment_size)

#define EXT2_GET_BLOCK_GROUP_COUNT(sb) (((sb)->total_blocks + (sb)->blocks_per_group - 1) / (sb)->blocks_per_group)

#define EXT2_GET_INODE_BLOCK_GROUP(sb, ino) ((ino - 1) / (sb)->inodes_per_group)
#define EXT2_GET_INODE_INDEX(sb, ino) ((ino - 1) % (sb)->inodes_per_group)

#define EXT2_SECTORS_PER_BLOCK(block_size, sector_size) ((block_size) / (sector_size))
#define EXT2_INODES_PER_BLOCK(block_size, inode_size) ((block_size) / (inode_size))











typedef struct {
    uint32_t    total_inodes;                // 0
    uint32_t    total_blocks;                // 4
    uint32_t    reserved_blocks;             // 8
    uint32_t    total_unallocated_blocks;    // 12
    uint32_t    total_unallocated_inodes;    // 16
    uint32_t    superblock_block_num;        // 20
    uint32_t    log2_block_size;             // 24
    uint32_t    log2_fragment_size;          // 28
    uint32_t    blocks_per_group;            // 32
    uint32_t    fragments_per_group;         // 36
    uint32_t    inodes_per_group;            // 40
    uint32_t    last_mtime;                  // 44
    uint32_t    last_wtime;                  // 48
    uint16_t    mount_count;                 // 52
    uint16_t    mount_allowed_count;         // 54
    uint16_t    ext2_signature;              // 56
    uint16_t    filesystem_state;            // 58
    uint16_t    minor_version;               // 60
    uint32_t    last_check;                  // 64
    uint32_t    last_forced_check;           // 68
    uint32_t    operating_system_id;         // 72
    uint32_t    major_version;               // 76
    uint16_t    reserved_uid_blocks;         // 80
    uint16_t    reserved_gid_blocks;         // 82

    // Extended fields for major_version >= 1
    uint32_t    first_non_reserved_inode;    // 84
    uint16_t    inode_structure_size;        // 88
    uint16_t    superblock_block_group;      // 90
    uint32_t    optional_features;           // 92
    uint32_t    required_features;           // 96
    uint32_t    non_supported_features;      // 100
    uint8_t     filesystem_id[16];           // 104
    char        volume_name[16];             // 120
    char        path_mount[64];              // 136
    uint32_t    compression_algorithm;       // 200
    uint8_t     num_prealloc_files;          // 204
    uint8_t     num_prealloc_dirs;           // 205
    uint16_t    reserved;                    // 206
    char        journal_id[16];              // 208
    uint32_t    journal_inode;               // 224
    uint32_t    journal_device;              // 228
    uint32_t    head_orphan_inode_list;      // 232

    uint8_t     __pad[1024 - 236];           // Padding to 1024 bytes
} ext2_superblock_t;

typedef struct {
    uint32_t    bitmap_block_address;               // Block address of block usage bitmap
    uint32_t    bitmap_inode_address;               // Block address of inode usage bitmap
    uint32_t    inode_table_start;                  // Starting block address of inode table
    uint16_t    unallocated_group_blocks;           // Number of unallocated group blocks in group
    uint16_t    unallocated_inode_blocks;           // Number of unallocated inode blocks in group
    uint16_t    num_directories;                    // Number of directories in group
    uint8_t     __pad[14];
} ext2_block_group_t;

typedef struct {
    uint16_t    type_and_permissions;  // 0x00
    uint16_t    user_id;               // 0x02
    uint32_t    lo_size_bytes;         // 0x04
    uint32_t    last_atime;            // 0x08
    uint32_t    ctime;                 // 0x0C
    uint32_t    last_mtime;            // 0x10
    uint32_t    dtime;                 // 0x14
    uint16_t    group_id;              // 0x18
    uint16_t    hard_link_count;       // 0x1A
    uint32_t    disk_sectors_count;    // 0x1C
    uint32_t    flags;                 // 0x20
    uint32_t    operating_system_specific; // 0x24
    uint32_t    blocks[15];            // 0x28
    uint32_t    generation_number;     // 0x64
    uint32_t    extended_attribute_block; // 0x68
    uint32_t    hi_size_bytes;         // 0x6C
    uint32_t    block_address_fragment; // 0x70
    uint8_t     operating_system_specific2[12]; // 0x74
    uint16_t    extra_inode_size;      // 0x80
    uint16_t    checksum_hi;           // 0x82
    uint32_t    reserved[2];           // 0x84, 0x88
} ext2_inode_t;

typedef struct {
    uint32_t    inode;
    uint16_t    entry_size;
    uint8_t     name_length;
    uint8_t     type;
    char        name[];
} ext2_dirent_t;









void ext2_init(void);
uint32_t ext2_get_block_size(void);
uint32_t ext2_get_fragment_size(void);
uint32_t ext2_get_blocks_per_group(void);
uint32_t ext2_get_inodes_per_group(void);
uint32_t ext2_get_first_data_block(void);
uint32_t ext2_lba_to_offset(uint64_t lba);
uint32_t ext2_offset_to_lba(uint32_t offset);
uint32_t ext2_get_block_group_count(void);
uint32_t ext2_get_inode_size(void);
uint32_t ext2_inode_get_block_group(uint32_t inode);
uint32_t ext2_get_inode_index(uint32_t inode);
ext2_inode_t *ext2_read_inode(vfs_superblock_t *vfs_sb, uint32_t inode_num);
void ext2_free_inode(ext2_inode_t *inode_ptr);
uint32_t ext2_get_inode_block_group(uint32_t inode);
uint32_t ext2_get_inode_index(uint32_t inode);
uint32_t ext2_get_sectors_per_block(void);
ext2_block_group_t *ext2_get_block_desc_for_inode(vfs_superblock_t *vfs_sb, uint32_t inode_num);
uint32_t ext2_get_inodes_per_block(void);
size_t ext2_get_inode_data_size(ext2_inode_t *inode);

vfs_dirent_t *ext2_mount(vfs_filesystem_type_t *fs, const char *path, uint32_t flags);
vfs_dirent_t *ext2_lookup(vfs_dirent_t *dir, const char *path);
int ext2_listdir(vfs_dirent_t *dir, const char *path);

#endif