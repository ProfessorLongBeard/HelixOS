#include <kstdio.h>
#include <kstdlib.h>
#include <list.h>






list_node_t *list_find(list_t *list, void *data) {
    list_foreach(item, list) {
        if (item->data == data) {
            return item;
        }
    }

    return NULL;
}