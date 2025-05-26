#include <kstdlib.h>
#include <spinlock.h>







void spinlock_init(spinlock_t *s) {
    assert(s != NULL);

    atomic_flag_clear(&s->lock);
}

void spinlock_acquire(spinlock_t *s) {
    assert(s != NULL);

    while(atomic_flag_test_and_set(&s->lock)) {
        __asm__ volatile("wfi\n\t");
    }

    __asm__ volatile("dmb ish\n\t" ::: "memory");
}

void spinlock_release(spinlock_t *s) {
    assert(s != NULL);

    __asm__ volatile("dmb ish\n\t" ::: "memory");

    atomic_flag_clear(&s->lock);
}