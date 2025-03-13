#include <kstring.h>
#include <framebuffer.h>







void fb_puts(const char *s) {
    flanterm_write(ft_ctx, s, strlen(s));
}

void fb_putc(char ch) {
    flanterm_write(ft_ctx, &ch, 1);
}