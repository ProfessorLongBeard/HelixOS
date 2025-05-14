#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include <spinlock.h>



typedef struct vfs_node     vfs_node_t;
typedef struct vfs_dirent   vfs_dirent_t;
typedef struct vfs_mount    vfs_mount_t;
typedef struct vfs_fs_opts  vfs_fs_opts_t;
typedef struct vfs_file     vfs_file_t;



#define VFS_MAX_NAME_LEN    256

#define SEEK_CURR       1
#define SEEK_END        2
#define SEEK_SET        0

#define VFS_TYPE_UNKNOWN    0x0     // Unknown file type
#define VFS_TYPE_FILE       0x1     // Regular file
#define VFS_TYPE_DIR        0x2     // Directory
#define VFS_TYPE_CHAR       0x3     // Character device
#define VFS_TYPE_BLK        0x4     // Block device
#define VFS_TYPE_FIFO       0x5     // FIFO device
#define VFS_TYPE_PIPE       0x6     // Pipe device
#define VFS_TYPE_SYM        0x20    // Symbolic link
#define VFS_TYPE_MNT        0x40    // Mount point

#define VFS_IFMT        0170000
#define VFS_IFDIR       0040000
#define VFS_IFCHR       0020000
#define VFS_IFBLK       0060000
#define VFS_IFREG       0100000
#define VFS_IFLINK      0120000
#define VFS_IFSOCK      0240000
#define VFS_IFIFO       0010000

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_ACCMODE       0x0003

#define O_CREAT         0x0100
#define O_EXCL          0x0200
#define O_TRUNC         0x0400
#define O_APPEND        0x0800

// TODO: Add more error codes when needed...
#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define EACCES         13
#define EEXIST         17
#define ENOTDIR        20
#define EISDIR         21
#define EINVAL         22
#define ENFILE         23
#define EMFILE         24
#define ENOSPC         28
#define EROFS          30
#define ENAMETOOLONG   36
#define ENOTEMPTY      39
#define ELOOP          40

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
    vfs_node_t      *(*fs_mount)(const char *path);
    vfs_node_t      *(*fs_lookup)(vfs_node_t *n, const char *name);
    int             (*fs_open)(vfs_node_t *node, uint32_t flags);
    int             (*fs_close)(vfs_node_t *node);
    int             (*fs_read)(vfs_node_t *node, void *buf, size_t length);
} vfs_fs_opts_t;

typedef struct vfs_file {
    vfs_node_t  *node;
    uint32_t    inode;
    uint32_t    type;
    uint32_t    mode;
    uint32_t    flags;
    uint64_t    offset;
    size_t      length;
    char        name[VFS_MAX_NAME_LEN];
} vfs_file_t;

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
    uint32_t        refcount;

    uint32_t        atime;
    uint32_t        mtime;
    uint32_t        ctime;

    struct vfs_node *link;      // For symlinks
    uint32_t        nlink;      // Number of links

    size_t          length;

    vfs_fs_opts_t   *fs_opts;
    void            *fs_private;

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
vfs_node_t *vfs_lookup(const char *path);
vfs_file_t *vfs_open(const char *path, uint32_t flags);
int vfs_close(vfs_file_t *fd);
int vfs_read(vfs_file_t *fd, void *buf, size_t length);

#endif