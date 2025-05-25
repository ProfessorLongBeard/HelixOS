#include <kstdio.h>
#include <kstdlib.h>
#include <list.h>








void list_delete(list_t *list, list_node_t *node) {
    if (!list || !node) {
        return;
    }

    if (node == list->list_head) {
        list->list_head = node->next;
    }

    if (node == list->list_tail) {
        list->list_tail = node->prev;
    }

    if (node->prev) {
        node->prev->next = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }

    list->list_count--;
}