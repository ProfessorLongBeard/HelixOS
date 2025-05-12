#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <fs/ext2/ext2.h>






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