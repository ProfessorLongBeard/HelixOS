#include <kstdio.h>
#include <kstdlib.h>
#include <list.h>





void list_append(list_t *list, list_node_t *node) {
    if (!list || !node) {
        return;
    }

    node->next = NULL;

    if (!list->list_tail) {
        list->list_head = node;
    } else {
        list->list_tail->next = node;
        node->prev = list->list_tail;
    }

    list->list_tail = node;
    list->list_count++;
}