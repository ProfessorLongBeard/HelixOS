#include <kstdio.h>
#include <kstdlib.h>
#include <list.h>






void list_destroy(list_t *list) {
    list_node_t *curr = NULL;


    if (!list) {
        return;
    }

    curr = list->list_head;

    while(curr != NULL) {
        list_node_t *next = curr->next;
        
        kfree(curr, sizeof(list_node_t));
        curr = next;
    }

    kfree(list, sizeof(list_t));
}