#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <limine.h>



__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

#endif