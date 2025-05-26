#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <mm/mm.h>
#include <list.h>






void list_insert(list_t *list, void *data) {
    list_node_t *list_node = NULL;


    if (!list || !data) {
        return;
    }

    list_node = kmalloc(sizeof(list_node_t));

    if (!list_node) {
        return;
    }

    list_node->data = data;
    list_node->next = NULL;
    list_node->prev = NULL;

    list_append(list, list_node);
}