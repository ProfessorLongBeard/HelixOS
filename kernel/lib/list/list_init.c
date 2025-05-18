#include <kstdio.h>
#include <kstring.h>
#include <kstdlib.h>
#include <list.h>







list_t *list_init(void) {
    list_t *l = NULL;


    l = kmalloc(sizeof(list_t));

    if (!l) {
        return NULL;
    }

    memset(l, 0, sizeof(list_t));

    l->list_count = 0;
    l->list_head = NULL;
    l->list_tail = NULL;

    return l;
}