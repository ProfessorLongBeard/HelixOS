#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <fs/vfs.h>
#include <fs/gpt.h>
#include <fs/ext2/ext2.h>
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



static spinlock_t s;
ext2_superblock_t *sb = NULL;


static vfs_fs_opts_t ext2_fs_opts = {
    .fs_mount = &ext2_mount,
    .fs_lookup = &ext2_lookup,
    .fs_open = &ext2_open
};



void ext2_init(void) {
    spinlock_init(&s);

    vfs_register("ext2fs", &ext2_fs_opts);
}

vfs_node_t *ext2_mount(const char *path) {
    uint32_t ret = 0;
    vfs_node_t *root = NULL;
    ext2_inode_t *ext2_root = NULL;
    uint32_t ext2_idx = gpt_partition_get_index("Rootfs");
    uint64_t ext2_offset = gpt_partition_get_offset(ext2_idx);



    if (!path) {
        return NULL;
    }

    sb = kmalloc(EXT2_SUPERBLOCK_SIZE);
    assert(sb != NULL);

    ret = virtio_blk_read((uint8_t *)sb, ext2_offset + 2, EXT2_SUPERBLOCK_SIZE);
    assert(ret == 0);

    root = kmalloc(sizeof(vfs_node_t));
    assert(root != NULL);

    strncpy(root->name, path, sizeof(root->name));

    ext2_root = ext2_read_inode(EXT2_ROOT_INODE);
    assert(ext2_root != NULL);

    root->inode = EXT2_ROOT_INODE;
    root->refcount = 1;

    root->type = (ext2_root->type_and_permissions & EXT2_TYPE_MASK);
    root->mode = (ext2_root->type_and_permissions & EXT2_PERM_MASK);

    root->uid = ext2_root->user_id;
    root->gid = ext2_root->group_id;

    root->link = NULL;
    root->nlink = 0;

    root->length = ext2_get_inode_data_size(ext2_root);

    root->ctime = ext2_root->ctime;
    root->atime = ext2_root->last_atime;
    root->mtime = ext2_root->last_mtime;

    root->parent = NULL;
    root->children = NULL;
    root->siblings = NULL;

    root->fs_private = (ext2_inode_t *)ext2_root;
    root->fs_opts = &ext2_fs_opts;

    root->type = VFS_TYPE_DIR;
    root->type |= VFS_TYPE_MNT;

    return root;
}

vfs_node_t *ext2_lookup(vfs_node_t *root, const char *path) {
    uint32_t ret = 0;
    char tmp_name[1024];
    uint8_t *tmp_dirent = NULL;
    vfs_node_t *node = NULL;
    ext2_inode_t *root_node = NULL, *tmp_node = NULL;
    ext2_dirent_t *dir = NULL;
    uint32_t block = 0, block_lba = 0;
    uint32_t offset = 0, block_size = ext2_get_block_size();
    uint32_t sectors_per_block = ext2_get_sectors_per_block();
    uint32_t ext2_idx = gpt_partition_get_index("Rootfs");
    uint64_t ext2_offset = gpt_partition_get_offset(ext2_idx);



    
    if (!root || !path) {
        return NULL;
    }

    root_node = (ext2_inode_t *)root->fs_private;
    assert(root_node != NULL);

    // TODO: Handle indirect nodes
    for (uint32_t i = 0; i < 12; i++) {
        if (root_node->blocks[i] == 0) {
            continue;
        }

        block = root_node->blocks[i];
        block_lba = block * sectors_per_block;
        break;
    }

    // TODO: Allocate with block size instead of sectors per block here?
    tmp_dirent = kmalloc(sectors_per_block);
    assert(tmp_dirent != NULL);

    ret = virtio_blk_read((uint8_t *)tmp_dirent, ext2_offset + block_lba, sectors_per_block);
    assert(ret == 0);

    while(offset <= block_size) {
        dir = (ext2_dirent_t *)(tmp_dirent + offset);
        assert(dir != NULL);
        
        if (dir->entry_size == 0) {
            // Need to break of no other entries to exist to avoid running into an infinite loop
            break;
        }

        memset(tmp_name, 0, sizeof(tmp_name));

        strncpy(tmp_name, dir->name, dir->name_length);
        tmp_name[dir->name_length] = '\0';

        // TODO: Handle additional paths...
        if (strcmp(tmp_name, path) == 0) {
            tmp_node = ext2_read_inode(dir->inode);
            assert(tmp_node != NULL);

            node = kmalloc(sizeof(vfs_node_t));
            assert(node != NULL);
            
            strncpy(node->name, tmp_name, sizeof(node->name));

            node->inode = dir->inode;

            node->uid = tmp_node->user_id;
            node->gid = tmp_node->group_id;

            node->type = (tmp_node->type_and_permissions & EXT2_TYPE_MASK);
            node->mode = (tmp_node->type_and_permissions & EXT2_PERM_MASK);

            node->refcount = 1;
           
            node->ctime = tmp_node->ctime;
            node->atime = tmp_node->last_atime;
            node->mtime = tmp_node->last_mtime;

            node->length = ext2_get_inode_data_size(tmp_node);

            // TODO: Check for symlinks
            node->link = NULL;
            node->nlink = 0;

            node->parent = NULL;
            node->children = NULL;
            node->siblings = NULL;

            node->fs_opts = &ext2_fs_opts;
            node->fs_private = (ext2_inode_t *)tmp_node;
            break;
        }

        offset += dir->entry_size;
    }

    if (!node) {
        return NULL;
    }

    kfree(tmp_dirent, sectors_per_block);
    return node;
}

int ext2_open(vfs_node_t *node, uint32_t flags) {
    ext2_inode_t *inode = NULL;


    if (!node) {
        return -EINVAL;
    }

    if (node->type == VFS_TYPE_DIR) {
        return -EISDIR;
    }

    inode = (ext2_inode_t *)node->fs_private;
    assert(inode != NULL);

    if ((flags & O_CREAT) || (flags & O_RDWR) || (flags & O_APPEND)) {
        return -EROFS;
    }

    // TODO: Check for exec permissions? And maybe only check for user permisisons for now?
    if (!(node->mode & EXT2_S_IRUSR) || !(node->mode & EXT2_S_IRGRP) || !(node->mode & EXT2_S_IROTH)) {
        return -EACCES;
    }

    return 0;
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

ext2_inode_t *ext2_read_inode(uint32_t inode_num) {
    uint32_t ret = 0;
    uint8_t *buf = NULL;
    ext2_inode_t *inode = NULL;
    uint32_t ext2_part_idx = gpt_partition_get_index("Rootfs");
    uint32_t ext2_part_offset = gpt_partition_get_offset(ext2_part_idx);
    uint32_t sectors_per_block = ext2_get_sectors_per_block();
    ext2_block_group_t *bgdt = NULL;




    spinlock_acquire(&s);

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
    spinlock_release(&s);
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

    spinlock_acquire(&s);

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
                spinlock_release(&s);
                break;
            }

            strncpy(tmp_name, dir->name, dir->name_length);
            tmp_name[dir->name_length] = '\0';

            printf("%s\n", tmp_name);

            offset += dir->entry_size;
        }
    }

    kfree((ext2_dirent_t *)tmp_dirent, block_size);
    spinlock_release(&s);
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