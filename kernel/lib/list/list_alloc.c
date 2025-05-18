#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <list.h>







list_node_t *list_node_alloc(void *data, size_t length) {
    list_node_t *node = NULL;


    
    if (!data) {
        return NULL;
    }

    node = kmalloc(sizeof(list_node_t));

    if (!node) {
        return NULL;
    }

    node->data = data;
    node->length = length;

    node->next = NULL;
    node->prev = NULL;
    
    return node;
}