#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stddef.h>




typedef struct list_node {
    void                *data;
    size_t              length;
    struct list_node    *next;
    struct list_node    *prev;
} list_node_t;

typedef struct {
    uint64_t        list_count;
    list_node_t     *list_head;
    list_node_t     *list_tail;
} list_t;

#define list_foreach(item, list) \
    for (list_node_t *item = (list)->list_head; item != NULL; item = item->next)






list_t *list_init(void);
void list_insert(list_t *list, void *data);
void list_append(list_t *list, list_node_t *node);
list_node_t *list_find(list_t *list, void *data);
void list_delete(list_t *list, list_node_t *node);
void list_remove(list_t *list, uint64_t idx);
void list_free(list_t *list);
list_node_t *list_pop(list_t *list);

#endif