#include <kstdio.h>
#include <kstdlib.h>
#include <mm/mm.h>
#include <list.h>







list_t *list_init(void) {
    list_t *list = NULL;

    list = kmalloc(sizeof(list_t));

    if (!list) {
        return NULL;
    }

    list->list_count = 0;
    list->list_head = NULL;
    list->list_tail = NULL;

    return list;
}