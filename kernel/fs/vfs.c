#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <fs/vfs.h>





static vfs_mount_t *vfs_root = NULL;
static vfs_fs_type_t *fs_types = NULL;








void vfs_init(void) {
    if (!vfs_root) {
        vfs_root = kmalloc(sizeof(vfs_mount_t));
        assert(vfs_root != NULL);

        spinlock_init(&vfs_root->s);
    }

    if (!fs_types) {
        fs_types = kmalloc(sizeof(vfs_fs_type_t));
        assert(fs_types != NULL);
    }
}


void vfs_mount(const char *path, const char *fs_type) {
    vfs_fs_type_t *fs_curr = fs_types;
    vfs_node_t *fs_root = NULL;


    if (!path || !fs_type) {
        return;
    }

    while(fs_curr != NULL) {
        if (strcmp(fs_curr->fs_type, fs_type) == 0) {
            fs_root = fs_curr->fs_opts->fs_mount(path);
            assert(fs_root != NULL);

            vfs_root->root = fs_root;
            
            printf("VFS: Mounted %s on %s\n", fs_curr->fs_type, fs_root->name);
            return;
        }
    }

    printf("VFS: %s not registered!\n", fs_type);
}

void vfs_register(const char *fs_type, vfs_fs_opts_t *opts) {
    vfs_fs_type_t *prev = fs_types;


    if (!fs_type || !opts) {
        return;
    }

    while(prev->next != NULL) {
        prev = prev->next;
    }

    strncpy(prev->fs_type, fs_type, sizeof(prev->fs_type));

    prev->fs_opts = opts;
    prev->next = NULL;

    printf("VFS: Registered %s!\n", prev->fs_type);
}

vfs_node_t *vfs_lookup(const char *path) {
    vfs_node_t *root = vfs_root->root, *node = NULL;
    char tmp_path[1024], *token = NULL;

    if (!path) {
        printf("VFS: Invalid path %s!\n", path);
        return NULL;
    }
    
    strncpy(tmp_path, path, sizeof(tmp_path));
    tmp_path[sizeof(tmp_path) - 1] = '\0';

    token = strtok(tmp_path, "/");

    while(token != NULL) {
        // TODO: Handle parent
        vfs_node_t *ch = root->children;

        while(ch != NULL) {
            if (strcmp(ch->name, token) == 0) {
                printf("VFS: Found cached node for %s!\n", token);

                node = ch;
                break;
            }

            ch = ch->siblings;
        }

        if (!ch) {
            printf("VFS: %s not cached! Creating...\n", token);

            node = root->fs_opts->fs_lookup(root, token);
            assert(node != NULL);

            if (!root->children) {
                root->children = node;
            } else {
                vfs_node_t *n = root->children;
                
                while(n->siblings != NULL) {
                    n = n->siblings;
                }

                n->siblings = node;
            }
        }

        root = node;
        token = strtok(NULL, "/");
    }

    if (!node) {
        printf("VFS: Failed to lookup %s!\n", path);
        return NULL;
    }

    return node;
}