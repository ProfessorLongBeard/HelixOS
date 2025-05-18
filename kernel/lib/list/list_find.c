#include <kstdio.h>
#include <kstring.h>
#include <kstdlib.h>
#include <list.h>







void *list_find(list_t *list, void *cookie) {
    list_node_t *node = NULL;


    if (!list || !cookie) {
        return NULL;
    }

    node = list->list_head;

    while(node != NULL) {
        if (node->data == cookie) {
            return node->data;
        }

        node = node->next;
    }

    return NULL;
}