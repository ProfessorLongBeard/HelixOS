#include <kstdio.h>
#include <kstring.h>
#include <kstdlib.h>
#include <list.h>








void list_push_front(list_t *list, void *data, size_t length) {
    list_node_t *node = NULL;


    if (!list || !data) {
        return;
    }

    node = list_node_alloc(data, length);

    if (!node) {
        return;
    }

    node->next = list->list_head;

    if (list->list_head) {
        list->list_head->prev = node;
    } else {
        list->list_tail = node;
    }

    list->list_head = node;
    list->list_count++;
}

void list_push_back(list_t *list, void *data, size_t length) {
    list_node_t *node = NULL;


    if (!list || !data) {
        return;
    }

    node = list_node_alloc(data, length);

    if (!node) {
        return;
    }

    node->prev = list->list_tail;

    if (list->list_tail) {
        list->list_tail->next = node;
    } else {
        list->list_head = node;
    }

    list->list_tail = node;
    list->list_count++;
}