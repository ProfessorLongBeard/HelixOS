#include <kstdio.h>
#include <kstdlib.h>
#include <mm/mm.h>
#include <list.h>








void list_destroy(list_t *list) {
    list_node_t *node = NULL;


    if (!list) {
        return;
    }

    node = list->list_head;

    while(node != NULL) {
        kfree(node->data, node->length);
        node = node->next;
    }
}