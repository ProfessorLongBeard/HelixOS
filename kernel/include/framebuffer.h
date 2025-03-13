#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <fb.h>
#include <flanterm.h>




extern struct flanterm_context *ft_ctx;
extern struct limine_framebuffer *fb;



void fb_init(void);
void fb_puts(const char *s);
void fb_putc(char ch);

#endif