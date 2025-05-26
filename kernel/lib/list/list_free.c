#include <kstdio.h>
#include <kstdlib.h>
#include <mm/mm.h>
#include <list.h>






void list_free(list_t *list) {
    list_node_t *node = NULL;


    if (!list) {
        return;
    }

    node = list->list_head;

    while(node != NULL) {
        list_node_t *n = node->next;

        kfree(node, sizeof(list_node_t));
        node = n;
    }
}