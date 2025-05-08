#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>



typedef struct vfs_node vfs_node_t;
typedef struct vfs_opts vfs_opts_t;
typedef struct vfs_dirent vfs_dirent_t;

#define VFS_MAX_NAME    256

#define VFS_TYPE_FIFO          0x1000
#define VFS_TYPE_CHAR_DEV      0x2000
#define VFS_TYPE_DIRECTORY     0x4000
#define VFS_TYPE_BLOCK_DEV     0x6000
#define VFS_TYPE_REG_FILE      0x8000
#define VFS_TYPE_SYMLINK       0xA000
#define VFS_TYPE_SOCKET        0xC000

#define VFS_S_SETUID           04000
#define VFS_S_SETGID           02000
#define VFS_S_STICKY           01000
#define VFS_S_IRUSR            00400
#define VFS_S_IWUSR            00200
#define VFS_S_IXUSR            00100
#define VFS_S_IRGRP            00040
#define VFS_S_IWGRP            00020
#define VFS_S_IXGRP            00010
#define VFS_S_IROTH            00004
#define VFS_S_IWOTH            00002
#define VFS_S_IXOTH            00001

#define VFS_DIR_TYPE_UNKNOWN       0
#define VFS_DIR_TYPE_REG_FILE      1
#define VFS_DIR_TYPE_DIR           2
#define VFS_DIR_TYPE_CHAR_DEV      3
#define VFS_DIR_TYPE_BLOCK_DEV     4
#define VFS_DIR_TYPE_FIFO          5
#define VFS_DIR_TYPE_SOCKET        6
#define VFS_DIR_TYPE_SYMLINK       7






typedef struct vfs_dirent {
    uint64_t            inode;
    uint64_t            type, mode;
    vfs_node_t          *node;
    char                name[VFS_MAX_NAME];
    struct vfs_dirent   *next;
} vfs_dirent_t;

typedef struct vfs_opts {
    void (*fs_mount)(const char *path, vfs_node_t *node);
    vfs_dirent_t  *(*fs_lookup)(vfs_node_t *node, const char *path);
} vfs_opts_t;

typedef struct vfs_node {
    char                name[VFS_MAX_NAME];
    uint64_t            inode;
    uint64_t            refcount;
    uint64_t            uid, gid;
    uint64_t            atime, ctime, mtime;
    uint64_t            type, mode;
    struct vfs_node     *parent;
    struct vfs_node     *children;
    vfs_opts_t          *fs_opts;
    void                *fs_private;
} vfs_node_t;

typedef struct vfs_root {
    char                fs_type[VFS_MAX_NAME];
    uint64_t            type;
    vfs_node_t          *root;
    struct vfs_root     *next;
} vfs_root_t;





void vfs_init(void);
vfs_node_t *vfs_node_alloc(void);
void vfs_register(const char *fs_type, vfs_opts_t *fs_opts);
void vfs_mount(const char *path, const char *fs_type);

#endif