#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <fs/vfs.h>
#include <fs/gpt.h>
#include <spinlock.h>
#include <fs/ext2/ext2.h>
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





static vfs_filesystem_type_t ext2_fs = {
    .fs_type = "ext2fs",
    .fs_type_list = NULL,
    .fs_mount = &ext2_mount,
    .fs_unmount = NULL
};

static vfs_filesystem_opts_t ext2_fs_opts = {
    .fs_lookup = &ext2_lookup,
    .fs_listdir = &ext2_listdir
};




void ext2_init(void) {
    vfs_register(&ext2_fs);
}

vfs_dirent_t *ext2_mount(vfs_filesystem_type_t *fs, const char *path, uint32_t flags) {
    uint32_t ret = 0;
    vfs_dirent_t *dir = NULL;
    vfs_inode_t *root_node = NULL;
    ext2_inode_t *ext2_root = NULL;
    ext2_superblock_t *ext2_sb = NULL;
    vfs_superblock_t *vfs_sb = NULL;
    uint32_t ext2_idx = gpt_partition_get_index("Rootfs");
    uint64_t ext2_offset = gpt_partition_get_offset(ext2_idx);


    

    if (!fs || !path) {
        return NULL;
    }

    ext2_sb = kmalloc(EXT2_SUPERBLOCK_SIZE);
    assert(ext2_sb != NULL);

    // EXT2 superblock on LBA 2
    ret = virtio_blk_read((uint8_t *)ext2_sb, ext2_offset + 2, EXT2_SUPERBLOCK_SIZE);
    assert(ret == 0);

    // Setup superblock information
    vfs_sb = kmalloc(sizeof(vfs_superblock_t));
    assert(vfs_sb != NULL);

    vfs_sb->sb_opts = NULL;
    vfs_sb->fs_type = &ext2_fs;
    vfs_sb->sb_length = EXT2_SUPERBLOCK_SIZE;
    vfs_sb->sb_private = (ext2_superblock_t *)ext2_sb;

    vfs_sb->sb_list = list_init();
    assert(vfs_sb->sb_list != NULL);
    
    dir = vfs_dirent_alloc();
    assert(dir != NULL);

    dir->d_inode = EXT2_ROOT_INODE;
    dir->d_sb = vfs_sb;

    ext2_root = ext2_read_inode(vfs_sb, EXT2_ROOT_INODE);
    assert(ext2_root != NULL);

    root_node = vfs_inode_alloc();
    assert(root_node != NULL);

    strncpy(root_node->name, path, sizeof(root_node->name));

    root_node->inode = EXT2_ROOT_INODE;
    root_node->uid = ext2_root->user_id;
    root_node->gid = ext2_root->group_id;

    root_node->length = EXT2_GET_INODE_DATA_SIZE(ext2_root);

    root_node->type = ext2_root->type_and_permissions & EXT2_TYPE_MASK;
    root_node->mode = ext2_root->type_and_permissions & EXT2_PERM_MASK;

    root_node->ctime = ext2_root->ctime;
    root_node->mtime = ext2_root->last_mtime;
    root_node->atime = ext2_root->last_atime;

    root_node->fs_opts = &ext2_fs_opts;
    root_node->fs_private = (ext2_inode_t *)ext2_root;

    root_node->device = NULL;

    root_node->refcount = 1;

    dir->d_node = root_node;

    return dir;
}

vfs_dirent_t *ext2_lookup(vfs_dirent_t *dir, const char *path) {
    uint32_t ret = 0;
    char *tmp_name = NULL;
    uint8_t *tmp_dirent = NULL;
    ext2_dirent_t *ext2_dir = NULL;
    ext2_inode_t *ext2_inode = NULL;
    vfs_dirent_t *vfs_dir = NULL;
    ext2_superblock_t *sb = NULL;
    vfs_superblock_t *vfs_sb = NULL;
    vfs_inode_t *vfs_inode = NULL;
    uint32_t ext2_idx = gpt_partition_get_index("Rootfs");
    uint64_t ext2_offset = gpt_partition_get_offset(ext2_idx);
    uint32_t block = 0, block_lba = 0, offset = 0;
    uint32_t sectors_per_block = 0, block_size = 0;





    if (!dir || !path) {
        return NULL;
    }

    vfs_sb = (vfs_superblock_t *)dir->d_sb;
    assert(vfs_sb != NULL);

    sb = (ext2_superblock_t *)vfs_sb->sb_private;
    assert(sb != NULL);

    block_size = EXT2_GET_BLOCK_SIZE(sb);
    sectors_per_block = EXT2_SECTORS_PER_BLOCK(block_size, 512);

    ext2_inode = ext2_read_inode(vfs_sb, dir->d_node->inode);
    assert(ext2_inode != NULL);

    // TODO: Handle indirect blocks
    for (uint32_t i = 0; i < 12; i++) {
        if (ext2_inode->blocks[i] == 0) {
            continue;
        }

        spinlock_acquire(&dir->d_lock);
        block = ext2_inode->blocks[i];
        block_lba = block * sectors_per_block;
        spinlock_release(&dir->d_lock);

        tmp_dirent = kmalloc(block_size);
        assert(tmp_dirent != NULL);

        ret = virtio_blk_read(tmp_dirent, ext2_offset + block_lba, sectors_per_block);
        assert(ret == 0);

        while(offset < block_size) {
            ext2_dirent_t *d = (ext2_dirent_t *)(tmp_dirent + offset);
            assert(d != NULL);

            if (d->entry_size == 0) {
                break;
            }

            tmp_name = kmalloc(d->name_length + 1);
            assert(tmp_name != NULL);

            strncpy(tmp_name, d->name, d->name_length);
            tmp_name[d->name_length] = '\0';

            if (strcmp(tmp_name, path) == 0) {
                ext2_dir = kmalloc(d->entry_size);
                assert(ext2_dir != NULL);

                memcpy(ext2_dir, d, d->entry_size);

                kfree(tmp_name, d->name_length + 1);
                break;
            }

            kfree(tmp_name, d->name_length + 1);
            offset += d->entry_size;
        }
    }

    if (!ext2_dir) {
        kfree(tmp_dirent, block_size);
        return NULL;
    }

    vfs_dir = vfs_dirent_alloc();
    assert(vfs_dir != NULL);

    strncpy(vfs_dir->d_name, ext2_dir->name, ext2_dir->name_length);
    vfs_dir->d_name[ext2_dir->name_length] = '\0';

    spinlock_acquire(&dir->d_lock);
    vfs_dir->d_inode = ext2_dir->inode;
    vfs_dir->d_type = ext2_dir->type;
    vfs_dir->d_sb = vfs_sb;
    spinlock_release(&dir->d_lock);

    vfs_inode = vfs_inode_alloc();
    assert(vfs_inode != NULL);

    spinlock_acquire(&dir->d_lock);
    vfs_inode->inode = vfs_dir->d_inode;
    vfs_inode->ctime = ext2_inode->ctime;
    vfs_inode->atime = ext2_inode->last_atime;
    vfs_inode->mtime = ext2_inode->last_mtime;

    vfs_inode->uid = ext2_inode->user_id;
    vfs_inode->gid = ext2_inode->group_id;
    vfs_inode->type = ext2_inode->type_and_permissions & EXT2_TYPE_MASK;
    vfs_inode->mode = ext2_inode->type_and_permissions & EXT2_PERM_MASK;

    vfs_inode->fs_opts = &ext2_fs_opts;
    vfs_inode->fs_private = (ext2_inode_t *)ext2_inode;

    vfs_dir->d_node  = vfs_inode;
    spinlock_release(&dir->d_lock);

    kfree(tmp_dirent, block_size);
    return vfs_dir;
}

int ext2_listdir(vfs_dirent_t *dir, const char *path) {
    uint32_t ret = 0;
    char *tmp_name = NULL;
    uint8_t *tmp_dirent = NULL;
    ext2_inode_t *ext2_inode = NULL;
    vfs_dirent_t *vfs_dir = NULL;
    ext2_superblock_t *sb = NULL;
    vfs_superblock_t *vfs_sb = NULL;
    vfs_inode_t *vfs_inode = NULL;
    uint32_t ext2_idx = gpt_partition_get_index("Rootfs");
    uint64_t ext2_offset = gpt_partition_get_offset(ext2_idx);
    uint32_t block = 0, block_lba = 0, offset = 0;
    uint32_t sectors_per_block = 0, block_size = 0;





    if (!dir || !path) {
        return -EINVAL;
    }

    vfs_sb = (vfs_superblock_t *)dir->d_sb;
    assert(vfs_sb != NULL);

    sb = (ext2_superblock_t *)vfs_sb->sb_private;
    assert(sb != NULL);

    block_size = EXT2_GET_BLOCK_SIZE(sb);
    sectors_per_block = EXT2_SECTORS_PER_BLOCK(block_size, 512);

    ext2_inode = ext2_read_inode(vfs_sb, dir->d_node->inode);
    assert(ext2_inode != NULL);

    // TODO: Handle indirect blocks
    for (uint32_t i = 0; i < 12; i++) {
        if (ext2_inode->blocks[i] == 0) {
            continue;
        }

        spinlock_acquire(&dir->d_lock);
        block = ext2_inode->blocks[i];
        block_lba = block * sectors_per_block;
        spinlock_release(&dir->d_lock);

        tmp_dirent = kmalloc(block_size);
        assert(tmp_dirent != NULL);

        ret = virtio_blk_read(tmp_dirent, ext2_offset + block_lba, sectors_per_block);
        assert(ret == 0);

        while(offset < block_size) {
            ext2_dirent_t *d = (ext2_dirent_t *)(tmp_dirent + offset);
            assert(d != NULL);

            if (d->entry_size == 0) {
                break;
            }

            tmp_name = kmalloc(d->name_length + 1);
            assert(tmp_name != NULL);

            strncpy(tmp_name, d->name, d->name_length);
            tmp_name[d->name_length] = '\0';

            printf("%s\n", tmp_name);

            kfree(tmp_name, d->name_length + 1);
            offset += d->entry_size;
        }
    }

    kfree(tmp_dirent, block_size);
    return 0;
}

ext2_block_group_t *ext2_get_block_desc_for_inode(vfs_superblock_t *vfs_sb, uint32_t inode_num) {
    uint32_t ret = 0;
    uint8_t *bgdt_tmp = NULL;
    ext2_block_group_t *group = NULL;
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);
    ext2_superblock_t *sb = (ext2_superblock_t *)vfs_sb->sb_private;
    uint32_t block_size = EXT2_GET_BLOCK_SIZE(sb);
    uint32_t bgdt_block = (block_size == 1024) ? 2 : 1;
    uint32_t desc_per_block = block_size / sizeof(ext2_block_group_t);
    uint32_t block_group = EXT2_GET_INODE_BLOCK_GROUP(sb, inode_num);
    uint32_t local_index = EXT2_GET_INODE_INDEX(sb, inode_num);
    uint32_t bgdt_block_idx = block_group / desc_per_block;
    uint32_t bgdt_offset = block_group % desc_per_block;
    uint32_t bgdt_block_num = bgdt_block + bgdt_block_idx;
    uint32_t bgdt_byte_offset = bgdt_block_num * block_size;
    uint32_t bgdt_lba = EXT2_OFFSET_TO_LBA(bgdt_byte_offset, 512); // TODO: Get block size from virtio driver!
    uint32_t sectors_per_block = EXT2_SECTORS_PER_BLOCK(block_size, 512);


    bgdt_tmp = kmalloc(block_size);
    assert(bgdt_tmp != NULL);

    ret = virtio_blk_read(bgdt_tmp, ext2_part_offset + bgdt_lba, sectors_per_block);
    assert(ret == 0);

    group = kmalloc(sizeof(ext2_block_group_t));
    assert(group != NULL);

    // Copy block group struct information
    memcpy(group, ((uint8_t *)bgdt_tmp + (bgdt_offset * sizeof(ext2_block_group_t))), sizeof(ext2_block_group_t));

    kfree(bgdt_tmp, sizeof(ext2_block_group_t));
    return group;
}

ext2_inode_t *ext2_read_inode(vfs_superblock_t *vfs_sb, uint32_t inode_num) {
    uint32_t ret = 0;
    uint8_t *buf = NULL;
    ext2_inode_t *inode = NULL;
    ext2_superblock_t *sb = (ext2_superblock_t *)vfs_sb->sb_private;
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);
    uint32_t inode_size = EXT2_GET_INODE_SIZE(sb);
    uint32_t block_size = EXT2_GET_BLOCK_SIZE(sb);
    uint32_t sectors_per_block = EXT2_SECTORS_PER_BLOCK(block_size, 512);
    ext2_block_group_t *bgdt = NULL;

    
    bgdt = ext2_get_block_desc_for_inode(vfs_sb, inode_num);
    assert(bgdt != NULL);

    uint32_t block_group = EXT2_GET_INODE_BLOCK_GROUP(sb, inode_num);
    uint32_t local_index = EXT2_GET_INODE_INDEX(sb, inode_num);
    uint32_t inode_table_block = bgdt->inode_table_start;
    uint32_t inodes_per_block = EXT2_INODES_PER_BLOCK(block_size, inode_size);
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
    memcpy(inode, ((uint8_t *)buf + offset_in_block), inode_size);
    
    kfree(buf, block_size);
    return inode;
}