#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stddef.h>
#include <mm/mm.h>




typedef struct list_node {
    void                *data;
    size_t              length;

    struct list_node    *next;
    struct list_node    *prev;
} list_node_t;

typedef struct {
    int             list_count;
    list_node_t     *list_head;
    list_node_t     *list_tail;
} list_t;

#define list_foreach(list, node) \
    for (list_node_t *node = (list)->list_head; node != NULL; node = node->next)









list_t *list_init(void);
void list_destroy(list_t *list);
void list_push_front(list_t *list, void *data, size_t length);
void list_push_back(list_t *list, void *data, size_t length);
void list_pop(list_t *list, list_node_t *node);
list_node_t *list_node_alloc(void *data, size_t length);
void *list_pop_front(list_t *list);
void *list_pop_tail(list_t *list);
void *list_find(list_t *list, void *cookie);

#endif