#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include <spinlock.h>
#include <list.h>




typedef struct vfs_inode vfs_inode_t;
typedef struct vfs_dirent vfs_dirent_t;
typedef struct vfs_superblock vfs_superblock_t;
typedef struct vfs_filesystem_type vfs_filesystem_type_t;
typedef struct vfs_filesystem_opts vfs_filesystem_opts_t;
typedef struct vfs_superblock_opts vfs_superblock_opts_t;


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

typedef struct vfs_superblock_opts {
} vfs_superblock_opts_t;
typedef struct vfs_filesystem_type {
	char					*fs_type;
	vfs_dirent_t			*(*fs_mount)(vfs_filesystem_type_t *fs, const char *path, uint32_t flags);
	void					(*fs_unmount)(vfs_superblock_t *sb);
	list_t					*fs_type_list;
} vfs_filesystem_type_t;

typedef struct vfs_superblock {
	vfs_dirent_t			*sb_root;
	vfs_filesystem_type_t	*fs_type;
	vfs_superblock_opts_t	*sb_opts;
	void					*sb_private;
	size_t					sb_length;
	list_t					*sb_list;
} vfs_superblock_t;

typedef struct vfs_filesystem_opts {
    vfs_dirent_t    *(*fs_lookup)(vfs_dirent_t *dir, const char *path);
	int				(*fs_listdir)(vfs_dirent_t *dir, const char *path);
} vfs_filesystem_opts_t;

typedef struct vfs_inode {
	spinlock_t				lock;
    uint32_t            	inode;
    uint32_t            	length;
    uint32_t            	uid, gid;
    uint32_t           		type, mode;
    uint32_t            	atime, mtime, ctime;
	int						refcount;
	vfs_filesystem_opts_t	*fs_opts;
    void                	*fs_private;
	void					*device;
	char					name[VFS_MAX_NAME_LEN];
} vfs_inode_t;

typedef struct vfs_dirent {
	spinlock_t			d_lock;
    uint32_t            d_inode;
    uint32_t            d_mode;
    uint32_t            d_type;
	vfs_inode_t			*d_node;
	struct vfs_dirent	*d_parent;
	list_t				*d_children;
	list_t				*d_subdirs;
	vfs_superblock_t	*d_sb;
    char                d_name[VFS_MAX_NAME_LEN];
} vfs_dirent_t;







void vfs_init(void);
void vfs_mount(const char *path, const char *fs_type, uint32_t flags);
void vfs_register(vfs_filesystem_type_t *fs);
void vfs_mount(const char *path, const char *fs_type, uint32_t flags);
char **vfs_split_path(const char *path, int *token_count);
void vfs_free_split_path(const char **path_tokens, int token_count);
vfs_dirent_t *vfs_lookup(vfs_dirent_t *root, const char *path);
vfs_inode_t *vfs_inode_alloc(void);
vfs_dirent_t *vfs_dirent_alloc(void);
int vfs_listdir(const char *path);

#endif