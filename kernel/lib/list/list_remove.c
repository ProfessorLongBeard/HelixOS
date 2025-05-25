#include <kstdio.h>
#include <kstdlib.h>
#include <list.h>






void list_remove(list_t *list, uint64_t idx) {
    uint64_t i = 0;
    list_node_t *node = NULL;


    if (idx > list->list_count) {
        return;
    }

    node = list->list_head;

    while(i < idx) {
        node = node->next;
        i++;
    }

    list_delete(list, node);
}