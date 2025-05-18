#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <list.h>








void list_pop(list_t *list, list_node_t *node) {
    if (!list || !node) {
        return;
    }

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->list_head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->list_tail = node->prev;
    }

    kfree(node, sizeof(list_node_t));
    list->list_count--;
}

void *list_pop_front(list_t *list) {
    list_node_t *node = NULL;
    void *data = NULL;


    if (!list || !list->list_head) {
        return NULL;
    }

    node = list->list_head;
    data = node->data;

    list->list_head = node->next;

    if (list->list_head) {
        list->list_head->prev = NULL;
    } else {
        list->list_tail = NULL;
    }

    kfree(node, sizeof(list_node_t));
    list->list_count--;

    return data;
}

void *list_pop_tail(list_t *list) {
    list_node_t *node = NULL;
    void *data = NULL;


    if (!list || !list->list_tail) {
        return NULL;
    }

    node = list->list_tail;
    data = node->data;

    list->list_tail = node->prev;

    if (list->list_tail) {
        list->list_tail->prev = NULL;
    } else {
        list->list_head = NULL;
    }

    kfree(node, sizeof(list_node_t));
    list->list_count--;

    return data;
}