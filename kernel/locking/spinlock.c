#include <kstdlib.h>
#include <spinlock.h>







void spinlock_init(spinlock_t *s) {
    assert(s != NULL);

    atomic_flag_clear(&s->lock);
}

void spinlock_acquire(spinlock_t *s) {
    assert(s != NULL);

    while(atomic_flag_test_and_set(&s->lock));
}

void spinlock_release(spinlock_t *s) {
    assert(s != NULL);

    atomic_flag_clear(&s->lock);
}