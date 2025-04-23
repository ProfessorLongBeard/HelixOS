#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm.h>
#include <fs/vfs.h>
#include <fs/gpt.h>
#include <fs/ext2.h>
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





static ext2_superblock_t *sb = NULL;








void ext2_init(void) {
    uint32_t ret = 0;
    uint64_t ext2_idx = gpt_partition_get_index("Rootfs");
    uint64_t ext2_offset = gpt_partition_get_offset(ext2_idx);


    sb = kmalloc(EXT2_SUPERBLOCK_SIZE);

    if (!sb) {
        printf("EXT2: Failed to allocate EXT2 superblock buffer! (out of memory)]\n");
        return;
    }

    // Read EXT2 superblock information (LBA 2)
    ret = virtio_blk_read((uint8_t *)sb, ext2_offset + 2, EXT2_SUPERBLOCK_SIZE);

    if (ret != 0) {
        printf("EXT2: Failed to read EXT2 superblock!\n");

        kfree((ext2_superblock_t *)sb, EXT2_SUPERBLOCK_SIZE);
    }

    if (sb->ext2_signature != EXT2_SIGNATURE) {
        printf("EXT2: Invalid EXT2 magic: 0x%lx\n", sb->ext2_signature);

        kfree((ext2_superblock_t *)sb, EXT2_SUPERBLOCK_SIZE);
        return;
    }

    uint32_t block_size = ext2_get_block_size();
    uint32_t sectors_per_block = ext2_get_sectors_per_block();

    ext2_inode_t *root_inode = ext2_read_inode(EXT2_ROOT_INODE);
    assert(root_inode != NULL);

    printf("Root inode UID:     %lu\n", root_inode->user_id);
    printf("Root inode GID:     %lu\n", root_inode->group_id);
    printf("Root inode type:    0x%lx\n", (root_inode->type_and_permissions & 0xF000));
    printf("Root inode perms:   0x%lx\n", (root_inode->type_and_permissions & 0x0FFF));
    printf("Root inode flags:   0x%lx\n", root_inode->flags);

    ext2_dirent_t *dirent = ext2_resolve_path(root_inode, "test.txt");

    ext2_inode_t *test_file_inode = ext2_read_inode(dirent->inode);
    assert(test_file_inode != NULL);

    printf("%s inode:   %lu\n", dirent->name, dirent->inode);
    printf("%s type:    %lu\n", dirent->name, dirent->type);
    printf("%s size:    %lu\n", dirent->name, dirent->entry_size);
    
    printf("File type:  0x%lx\n", test_file_inode->type_and_permissions & 0xF000);
    printf("File perms: 0x%lx\n", test_file_inode->type_and_permissions & 0x0FFF);
    printf("File UID:   %lu\n", test_file_inode->user_id);
    printf("File GID:   %lu\n", test_file_inode->group_id);

    uint32_t test_file_block = test_file_inode->blocks[0];
    uint32_t test_file_lba = test_file_block * sectors_per_block;

    uint8_t *test = kmalloc(block_size);
    virtio_blk_read(test, ext2_offset + test_file_lba, block_size);

    printf("%s: %s\n", dirent->name, test);
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

    kfree(bgdt_tmp, block_size);
    return group;
}

ext2_dirent_t *ext2_resolve_path(ext2_inode_t *inode, const char *name) {
    uint32_t ret = 0;
    char tmp_name[256] = {0};
    uint8_t *tmp_dirent = NULL;
    ext2_dirent_t *dirent = NULL;
    uint32_t offset = 0, block_size = ext2_get_block_size();
    uint32_t sectors_per_block = ext2_get_sectors_per_block();
    uint32_t indirect_block = 0, indirect_block_lba = 0;
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);



    if (!inode || !name) {
        return NULL;
    }

    // TODO: Use other blocks for larger sizes
    indirect_block = inode->blocks[0];
    indirect_block_lba = indirect_block * sectors_per_block;

    tmp_dirent = kmalloc(1024);
    assert(tmp_dirent != NULL);

    ret = virtio_blk_read(tmp_dirent, ext2_part_offset + indirect_block_lba, sectors_per_block);
    assert(ret == 0);

    while(offset <= block_size) {
        ext2_dirent_t *dir = (ext2_dirent_t *)(tmp_dirent + offset);
        assert(dir != NULL);

        if (dir->entry_size == 0) {
            break;
        }

        printf("%s\n", dir->name);

        strncpy(tmp_name, dir->name, dir->name_length);
        tmp_name[dir->name_length] = '\0';

        if (strcmp(dir->name, name) == 0) {
            dirent = kmalloc(dir->entry_size);
            assert(dirent != NULL);

            // TODO: Find better way to do this maybe...?
            memcpy(dirent, dir, dir->entry_size);
            break;
        }

        offset += dir->entry_size;
    }

    return dirent;
}

ext2_inode_t *ext2_read_inode(uint32_t inode_num) {
    uint32_t ret = 0;
    uint8_t *buf = NULL;
    ext2_inode_t *inode = NULL;
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);
    uint32_t sectors_per_block = ext2_get_sectors_per_block();

    ext2_block_group_t *bgdt = ext2_get_block_desc_for_inode(inode_num);
    assert(bgdt != NULL);

    uint32_t inode_size = ext2_get_inode_size();
    uint32_t block_size = ext2_get_block_size();
    uint32_t block_group = ext2_get_inode_block_group(inode_num);
    uint32_t local_index = ext2_get_inode_index(inode_num);

    uint32_t inode_table_block = bgdt->inode_table_start;

    uint32_t inodes_per_block = block_size / inode_size;
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
    return inode;
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
    return 1024 << sb->log2_block_size;
}

uint32_t ext2_get_fragment_size(void) {
    return 1024 << sb->log2_fragment_size;
}

uint32_t ext2_get_blocks_per_group(void) {
    return sb->blocks_per_group;
}

uint32_t ext2_get_inodes_per_group(void) {
    return sb->inodes_per_group;
}

uint32_t ext2_get_first_data_block(void) {
    return sb->superblock_block_num;
}

uint32_t ext2_get_block_group_count(void) {
    return (sb->total_blocks + sb->blocks_per_group - 1) / sb->blocks_per_group;
}

uint32_t ext2_get_inode_size(void) {
    return sb->inode_structure_size;
}

uint32_t ext2_get_inode_block_group(uint32_t inode) {
    return (inode - 1) / sb->inodes_per_group;
}

uint32_t ext2_get_inode_index(uint32_t inode) {
    return (inode - 1) % sb->inodes_per_group;
}