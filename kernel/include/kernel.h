#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <limine.h>



extern uint64_t __kernel_start[];
extern uint64_t __kernel_end[];

extern uint64_t __stext[];
extern uint64_t __etext[];

extern uint64_t __srodata[];
extern uint64_t __erodata[];

extern uint64_t __sdata[];
extern uint64_t __edata[];

extern uint64_t __sbss[];
extern uint64_t __ebss[];

extern uint64_t __slimine_requests[];
extern uint64_t __elimine_requests[];

extern uint64_t __sstack[];
extern uint64_t __estack[];

#endif