#ifndef SPINLOCK_H
#define SPINLOCK_H


#include <stdbool.h>
#include <stdatomic.h>




typedef struct {
    atomic_bool *lock;
} spinlock_t;



void spinlock_init(spinlock_t *s);
void spinlock_acquire(spinlock_t *s);
void spinlock_release(spinlock_t *s);

#endif