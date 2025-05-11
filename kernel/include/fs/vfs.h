#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include <spinlock.h>



typedef struct vfs_node     vfs_node_t;
typedef struct vfs_dirent   vfs_dirent_t;
typedef struct vfs_mount    vfs_mount_t;
typedef struct vfs_fs_opts  vfs_fs_opts_t;



#define VFS_MAX_NAME_LEN    256

#define PATH_SEP        '/'
#define PATH_SEP_STR    "/"

#define PATH_DOT        "."
#define PATH_UP         ".."

#define VFS_TYPE_FILE   0x01    // Regular file
#define VFS_TYPE_DIR    0x02    // Directory
#define VFS_TYPE_CHAR   0x04    // Character device
#define VFS_TYPE_BLK    0x08    // Block device
#define VFS_TYPE_PIPE   0x10    // Pipe device
#define VFS_TYPE_SYM    0x20    // Symbolic link
#define VFS_TYPE_MNT    0x40    // Mount point

#define VFS_IFMT        0170000
#define VFS_IFDIR       0040000
#define VFS_IFCHR       0020000
#define VFS_IFBLK       0060000
#define VFS_IFREG       0100000
#define VFS_IFLINK      0120000
#define VFS_IFSOCK      0240000
#define VFS_IFIFO       0010000

struct stat {
    uint16_t  st_dev;
	uint16_t  st_ino;
	uint32_t  st_mode;
	uint16_t  st_nlink;
	uint16_t  st_uid;
	uint16_t  st_gid;
	uint16_t  st_rdev;
	uint32_t  st_size;
	uint32_t  st_atime;
	uint32_t  __unused1;
	uint32_t  st_mtime;
	uint32_t  __unused2;
	uint32_t  st_ctime;
	uint32_t  __unused3;
};

typedef struct vfs_fs_opts {
    vfs_dirent_t    *(*fs_readdir)(vfs_node_t *n, uint32_t inode);
    vfs_node_t      *(*fs_finddir)(vfs_node_t *n, const char *name);
} vfs_fs_opts_t;

typedef struct vfs_dirent {
    uint32_t    inode;
    char        name[VFS_MAX_NAME_LEN];
} vfs_dirent_t;

typedef struct vfs_node {
    void            *device;    // Device object

    uint32_t        inode;      // Inode number
    uint32_t        mask;       // Permission mask
    uint32_t        uid;        // Owner user ID
    uint32_t        gid;        // Owner group ID
    uint32_t        type;       // Flags (node, etc)
    uint32_t        mode;       // Permission flags

    uint32_t        atime;
    uint32_t        mtime;
    uint32_t        ctime;

    struct vfs_node *link;      // For symlinks
    uint32_t        nlink;      // Number of links

    size_t          length;

    vfs_fs_opts_t   *fs_opts;

    char            name[VFS_MAX_NAME_LEN];

    struct vfs_node *parent;
    struct vfs_node *children;
    struct vfs_node *siblings;
} vfs_node_t;

typedef struct vfs_fs_type {
    char                fs_type[VFS_MAX_NAME_LEN];
    vfs_fs_opts_t       *fs_opts;
    struct vfs_fs_type *next;
} vfs_fs_type_t;

typedef struct vfs_mount {
    spinlock_t  s;
    vfs_node_t  *root;
} vfs_mount_t;




void vfs_init(void);
void vfs_register(const char *fs_type, vfs_fs_opts_t *opts);
void vfs_mount(const char *path, const char *fs_type);

#endif