#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <fs/vfs.h>





static list_t *fs_types = NULL;
static list_t *vfs_mounts = NULL;
static vfs_dirent_t *vfs_root = NULL;







void vfs_init(void) {
    fs_types = list_init();
    assert(fs_types != NULL);

    vfs_mounts = list_init();
    assert(vfs_mounts != NULL);
}

void vfs_register(vfs_filesystem_type_t *fs) {
    if (!fs) {
        return;
    }

    list_insert(fs_types, (vfs_filesystem_type_t *)fs);
}

void vfs_mount(const char *path, const char *fs_type, uint32_t flags) {
    vfs_dirent_t *curr = vfs_root;
    vfs_dirent_t *root = NULL;
    vfs_filesystem_type_t *fs = NULL;




    if (!path || !fs_type) {
        return;
    }

    list_foreach(item, fs_types) {
        vfs_filesystem_type_t *f = (vfs_filesystem_type_t *)item->data;
        assert(f != NULL);

        if (strcmp(f->fs_type, fs_type) == 0) {
            fs = f;
            break;
        }
    }

    if (!fs) {
        printf("VFS: Filesystem %s unregistered!\n", fs_type);
        return;
    }

    if (!fs->fs_mount) {
        printf("VFS: Filesystem %s has no fs_mount()!\n", fs_type);
        return;
    }

    if (path[0] == '/' && path[1] == '\0') {
        // Mount filesystem on rootfs
        
        root = fs->fs_mount(fs, path, flags);

        if (!root) {
            printf("VFS: Filesystem fs_mount() failed!\n");
            return;
        }

        if (!vfs_root) {
            vfs_root = root;

            printf("VFS: Mounted %s on %s\n", fs->fs_type, path);
            return;
        } else {
            // Don't overwrite current rootfs
            printf("VFS: rootfs already mounted on %s\n", path);
            return;
        }
    }
}

char **vfs_split_path(const char *path, int *token_count) {
    int num_tokens = -1, i = 0;
    char **tokens = NULL, *token = NULL, *tmp_path = NULL, *save_ptr = NULL;
    size_t path_length = -1;



    if (!path || !token_count) {
        return NULL;
    }

    num_tokens = strtok_count(path);

    if (num_tokens == 0) {
        return NULL;
    }

    path_length = strlen(path) + 1;

    tmp_path = kmalloc(path_length);
    assert(tmp_path != NULL);

    strncpy(tmp_path, path, path_length);

    tokens = kmalloc(num_tokens);
    assert(tokens != NULL);

    token = strtok_r(tmp_path, "/", &save_ptr);
    assert(token != NULL);

    while(token != NULL && i <= num_tokens) {
        tokens[i++] = strdup(token);
        
        token = strtok_r(tmp_path, "/", &save_ptr);
    }

    *token_count = num_tokens;
    kfree(tmp_path, path_length);
    return tokens;
}

void vfs_free_split_path(const char **path_tokens, int token_count) {
    char **tokens = NULL;


    if (!path_tokens || token_count == 0) {
        return;
    }

    tokens = (char **)path_tokens;

    for (int i = 0; i < token_count; i++) {
        char *token = (char *)tokens[i];

        if (!token) {
            continue;
        }

        kfree(token, strlen(token));
    }

    kfree(path_tokens, token_count);
}

vfs_dirent_t *vfs_lookup(vfs_dirent_t *root, const char *path) {
    vfs_dirent_t *curr = root;
    int path_token_count = 0;
    char **path_tokens = NULL;


    if (!root || !path) {
        return NULL;
    }

    path_tokens = vfs_split_path(path, &path_token_count);

    if (!path_tokens && path_token_count == 0) {
        return root;
    }

    for (int i = 0; i < path_token_count; i++) {
        char *token = (char *)path_tokens[i];
        vfs_dirent_t *child_dir = NULL;

        list_foreach(item, curr->d_children) {
            vfs_dirent_t *ch = (vfs_dirent_t *)item->data;
            assert(ch != NULL);

            if (strcmp(ch->d_node->name, token) == 0) {
                child_dir = ch;
                break;
            }
        }

        if (!child_dir) {
            child_dir = curr->d_node->fs_opts->fs_lookup(curr, token);
            assert(child_dir != NULL);

            spinlock_acquire(&curr->d_lock);
            list_insert(curr->d_children, (vfs_dirent_t *)child_dir);
            spinlock_release(&curr->d_lock);
        }

        curr = child_dir;
    }

    vfs_free_split_path((const char **)path_tokens, path_token_count);
    return curr;
}

vfs_inode_t *vfs_inode_alloc(void) {
    vfs_inode_t *inode = NULL;


    inode = kmalloc(sizeof(vfs_inode_t));
    assert(inode != NULL);

    spinlock_init(&inode->lock);

    inode->device = NULL;
    inode->fs_opts = NULL;
    inode->fs_private = NULL;

    return inode;
}

vfs_dirent_t *vfs_dirent_alloc(void) {
    vfs_dirent_t *dirent = NULL;


    dirent = kmalloc(sizeof(vfs_dirent_t));
    assert(dirent != NULL);
    
    spinlock_init(&dirent->d_lock);

    dirent->d_children = list_init();
    assert(dirent->d_children != NULL);

    dirent->d_subdirs = list_init();
    assert(dirent->d_subdirs != NULL);

    dirent->d_node = NULL;
    dirent->d_parent = NULL;
    dirent->d_sb = NULL;
    return dirent;
}

int vfs_listdir(const char *path) {
    int ret = 0;
    vfs_dirent_t *curr = vfs_root, *dir = NULL;


    if (!path) {
        return -EINVAL;
    }

    dir = vfs_lookup(curr, path);
    assert(dir != NULL);

    if (!dir->d_node->fs_opts || !dir->d_node->fs_opts->fs_listdir) {
        return -ENOENT;
    }

    ret = dir->d_node->fs_opts->fs_listdir(dir, path);

    if (ret < 0) {
        return ret;
    }

    return 0;
}