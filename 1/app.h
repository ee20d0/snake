#ifndef APP_H
#define APP_H
#include <linux.h>

/* String */
void reverse(char *s);
void utoa(unsigned u, char *s);
int strlen(char *s);
int strcmp(const char *s, const char *t);
int strarg(const char *s, char **argv);
void fdputs(int fd, char *s);
void puts(char *s);

#endif