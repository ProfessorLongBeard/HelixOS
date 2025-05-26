#include <kstdio.h>
#include <kstdlib.h>
#include <list.h>





list_node_t *list_pop(list_t *list) {
    list_node_t *node = NULL;


    if (!list) {
        return NULL;
    }

    node = list->list_tail;

    if (!node) {
        return NULL;
    }

    list_delete(list, list->list_tail);

    return node;
}