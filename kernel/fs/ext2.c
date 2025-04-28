#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <fs/vfs.h>
#include <fs/gpt.h>
#include <fs/ext2.h>
#include <spinlock.h>
#include <devices/virtio/virtio.h>
#include <devices/virtio/virtio_blk.h>





/*
    How To Read An Inode:
    - Read the Superblock to find the size of each block, the number of blocks per group, number Inodes per group, and the starting block of the first group (Block Group Descriptor Table).
    - Determine which block group the inode belongs to.
    - Read the Block Group Descriptor corresponding to the Block Group which contains the inode to be looked up.
    - From the Block Group Descriptor, extract the location of the block group's inode table.
    - Determine the index of the inode in the inode table.
    - Index the inode table (taking into account non-standard inode size).
    - Directory entry information and file contents are located within the data blocks that the Inode points to.

    How To Read the Root Directory
    The root directory's inode is defined to always be 2. Read/parse the contents of inode 2.
 */





 
static ext2_t *fd;






void ext2_init(void) {
    uint32_t ret = 0;
    uint64_t ext2_idx = gpt_partition_get_index("Rootfs");
    uint64_t ext2_offset = gpt_partition_get_offset(ext2_idx);


    fd = kmalloc(sizeof(ext2_t));
    assert(fd != NULL);

    fd->sb = kmalloc(EXT2_SUPERBLOCK_SIZE);

    if (!fd->sb) {
        printf("EXT2: Failed to allocate EXT2 superblock buffer! (out of memory)]\n");
        return;
    }

    spinlock_init(&fd->s);

    // Read EXT2 superblock information (LBA 2)
    ret = virtio_blk_read((uint8_t *)fd->sb, ext2_offset + 2, EXT2_SUPERBLOCK_SIZE);
    assert(ret == 0);

    if (fd->sb->ext2_signature != EXT2_SIGNATURE) {
        printf("EXT2: Invalid EXT2 magic: 0x%lx\n", fd->sb->ext2_signature);

        kfree((ext2_superblock_t *)fd->sb, EXT2_SUPERBLOCK_SIZE);
        return;
    }

    fd->root = ext2_read_inode(EXT2_ROOT_INODE);
    assert(fd->root != NULL);

    // List contents of "/"
    printf("\nContents of: /\n");
    ext2_list_dir(fd->root);

    ext2_dirent_t *dev_dir = ext2_lookup(fd->root, "/dev");
    ext2_inode_t *dev_dir_inode = ext2_read_inode(dev_dir->inode);

    // List contents of "/dev"
    printf("\nContents of: /dev\n");
    ext2_list_dir(dev_dir_inode);

    ext2_dirent_t *test_file = ext2_lookup(fd->root, "/test.txt");
    ext2_inode_t *test_file_inode = ext2_read_inode(test_file->inode);

    uint32_t test_file_block = test_file_inode->blocks[0];
    uint32_t test_file_data_lba = test_file_block * ext2_get_sectors_per_block();

    size_t test_file_size = ext2_get_inode_data_size(test_file_inode);

    uint8_t *test_raw = kmalloc(ext2_get_block_size());
    virtio_blk_read(test_raw, ext2_offset + test_file_data_lba, ext2_get_block_size());

    printf("\nContents of: %s (size: %lu)\n", test_file->name, test_file_size);
    printf("%s\n", test_raw);
}

ext2_block_group_t *ext2_get_block_desc_for_inode(uint32_t inode_num) {
    uint32_t ret = 0;
    uint8_t *bgdt_tmp = NULL;
    ext2_block_group_t *group = NULL;
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);
    uint32_t block_size = ext2_get_block_size();
    uint32_t bgdt_block = (block_size == 1024) ? 2 : 1;
    uint32_t desc_per_block = block_size / sizeof(ext2_block_group_t);
    uint32_t block_group = ext2_get_inode_block_group(inode_num);
    uint32_t local_index = ext2_get_inode_index(inode_num);
    uint32_t bgdt_block_idx = block_group / desc_per_block;
    uint32_t bgdt_offset = block_group % desc_per_block;

    uint32_t bgdt_block_num = bgdt_block + bgdt_block_idx;
    uint32_t bgdt_byte_offset = bgdt_block_num * block_size;

    uint32_t bgdt_lba = ext2_offset_to_lba(bgdt_byte_offset);
    uint32_t sectors_per_block = ext2_get_sectors_per_block();

    bgdt_tmp = kmalloc(block_size);
    assert(bgdt_tmp != NULL);

    ret = virtio_blk_read(bgdt_tmp, ext2_part_offset + bgdt_lba, sectors_per_block);
    assert(ret == 0);

    group = kmalloc(sizeof(ext2_block_group_t));
    assert(group != NULL);

    // Copy block group struct information
    memcpy(group, bgdt_tmp + (bgdt_offset * sizeof(ext2_block_group_t)), sizeof(ext2_block_group_t));

    kfree(bgdt_tmp, sizeof(ext2_block_group_t));
    return group;
}

ext2_dirent_t *ext2_lookup(ext2_inode_t *inode, const char *name) {
    uint32_t ret = 0;
    char tmp_name[256] = {0}, tmp2[1024] = {0}, *ptr = NULL;
    uint8_t *tmp_dirent = NULL;
    ext2_dirent_t *dirent = NULL;
    uint32_t block = 0, block_lba = 0;
    uint32_t offset = 0, block_size = ext2_get_block_size();
    uint32_t sectors_per_block = ext2_get_sectors_per_block();
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);



    if (!inode || !name) {
        return NULL;
    }

    spinlock_acquire(&fd->s);

    ptr = (char *)name;

    if (*ptr == '/') {
        // Skip leading slash
        ptr++;
    }

    // TODO: Use other blocks if needed
    block = inode->blocks[0];
    block_lba = block * sectors_per_block;

    tmp_dirent = kmalloc(1024);
    assert(tmp_dirent != NULL);

    ret = virtio_blk_read(tmp_dirent, ext2_part_offset + block_lba, sectors_per_block);
    assert(ret == 0);

    while(offset <= block_size) {
        ext2_dirent_t *dir = (ext2_dirent_t *)(tmp_dirent + offset);
        assert(dir != NULL);

        if (dir->entry_size == 0) {
            spinlock_release(&fd->s);
            break;
        }

        strncpy(tmp_name, dir->name, dir->name_length);
        tmp_name[dir->name_length] = '\0';

        if (strcmp(tmp_name, ptr) == 0) {
            dirent = kmalloc(dir->entry_size);
            assert(dirent != NULL);

            memcpy(dirent, dir, dir->entry_size);
            break;
        }

        offset += dir->entry_size;
    }

    spinlock_release(&fd->s);
    return dirent;
}

ext2_inode_t *ext2_read_inode(uint32_t inode_num) {
    uint32_t ret = 0;
    uint8_t *buf = NULL;
    ext2_inode_t *inode = NULL;
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);
    uint32_t sectors_per_block = ext2_get_sectors_per_block();
    ext2_block_group_t *bgdt = NULL;




    spinlock_acquire(&fd->s);

    bgdt = ext2_get_block_desc_for_inode(inode_num);
    assert(bgdt != NULL);

    uint32_t inode_size = ext2_get_inode_size();
    uint32_t block_size = ext2_get_block_size();
    uint32_t block_group = ext2_get_inode_block_group(inode_num);
    uint32_t local_index = ext2_get_inode_index(inode_num);

    uint32_t inode_table_block = bgdt->inode_table_start;

    uint32_t inodes_per_block = ext2_get_inodes_per_block();
    uint32_t block_index = local_index / inodes_per_block;
    uint32_t offset_in_block = (local_index % inodes_per_block) * inode_size;
    uint32_t inode_block_lba = ext2_part_offset + ((inode_table_block + block_index) * sectors_per_block);

    buf = kmalloc(block_size);
    assert(buf != NULL);

    ret = virtio_blk_read(buf, inode_block_lba, block_size);
    assert(ret == 0);

    inode = kmalloc(inode_size);
    assert(inode != NULL);

    // Copy inode data
    memcpy(inode, buf + offset_in_block, inode_size);

    kfree(buf, block_size);
    spinlock_release(&fd->s);
    return inode;
}

void ext2_list_dir(ext2_inode_t *inode) {
    uint32_t ret = 0;
    char tmp_name[256] = {0}, tmp2[1024] = {0}, *ptr = NULL;
    uint8_t *tmp_dirent = NULL;
    ext2_dirent_t *dirent = NULL;
    uint32_t block = 0, block_lba = 0;
    uint32_t offset = 0, block_size = ext2_get_block_size();
    uint32_t sectors_per_block = ext2_get_sectors_per_block();
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);



    if (!inode) {
        return;
    }

    spinlock_acquire(&fd->s);

    for (uint32_t i = 0; i < 12; i++) {
        if (inode->blocks[i] == 0) {
            continue;
        }

        offset = 0;
 
        block = inode->blocks[i];
        block_lba = block * sectors_per_block;

        tmp_dirent = kmalloc(block_size);
        assert(tmp_dirent != NULL);

        ret = virtio_blk_read(tmp_dirent, ext2_part_offset + block_lba, sectors_per_block);
        assert(ret == 0);

        while(offset < block_size) {
            ext2_dirent_t *dir = (ext2_dirent_t *)(tmp_dirent + offset);
            assert(dir != NULL);

            if (dir->entry_size == 0) {
                spinlock_release(&fd->s);
                break;
            }

            strncpy(tmp_name, dir->name, dir->name_length);
            tmp_name[dir->name_length] = '\0';

            printf("%s\n", tmp_name);

            offset += dir->entry_size;
        }
    }

    kfree((ext2_dirent_t *)tmp_dirent, block_size);
    spinlock_release(&fd->s);
}

size_t ext2_get_inode_data_size(ext2_inode_t *inode) {
    if (!inode) {
        return -1;
    }

    return ((size_t)inode->hi_size_bytes << 32) | inode->lo_size_bytes;
}

uint32_t ext2_get_inodes_per_block(void) {
    uint32_t block_size = ext2_get_block_size();
    uint32_t inode_size = ext2_get_inode_size();
    uint32_t inodes_per_block = block_size / inode_size;

    return inodes_per_block;
}

uint32_t ext2_get_sectors_per_block(void) {
    uint32_t sector_size = 512; // TODO: Use virtio config to get this instead!
    uint32_t block_size = ext2_get_block_size();
    uint32_t sectors_per_block = block_size / sector_size;

    return sectors_per_block;
}

void ext2_free_inode(ext2_inode_t *inode_ptr) {
    uint32_t inode_size = ext2_get_inode_size();

    if (!inode_ptr) {
        return;
    }

    kfree((ext2_inode_t *)inode_ptr, inode_size);
}

uint32_t ext2_lba_to_offset(uint64_t lba) {
    return lba * 512;
}

uint32_t ext2_offset_to_lba(uint32_t offset) {
    return offset / 512;
}

uint32_t ext2_get_block_size(void) {
    return 1024 << fd->sb->log2_block_size;
}

uint32_t ext2_get_fragment_size(void) {
    return 1024 << fd->sb->log2_fragment_size;
}

uint32_t ext2_get_blocks_per_group(void) {
    return fd->sb->blocks_per_group;
}

uint32_t ext2_get_inodes_per_group(void) {
    return fd->sb->inodes_per_group;
}

uint32_t ext2_get_first_data_block(void) {
    return fd->sb->superblock_block_num;
}

uint32_t ext2_get_block_group_count(void) {
    return (fd->sb->total_blocks + fd->sb->blocks_per_group - 1) / fd->sb->blocks_per_group;
}

uint32_t ext2_get_inode_size(void) {
    return fd->sb->inode_structure_size;
}

uint32_t ext2_get_inode_block_group(uint32_t inode) {
    return (inode - 1) / fd->sb->inodes_per_group;
}

uint32_t ext2_get_inode_index(uint32_t inode) {
    return (inode - 1) % fd->sb->inodes_per_group;
}