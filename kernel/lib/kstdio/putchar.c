#include <kstdio.h>
#include <kstring.h>
#include <framebuffer.h>






void putchar_(char ch) {
    fb_putc(ch);
}