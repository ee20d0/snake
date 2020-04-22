#ifndef LINUX_H
#define LINUX_H

#include <types.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/personality.h>
#include <linux/errno.h>
#include <asm-generic/ioctls.h>

#define NULL	((void *)0)
#define memset	__builtin_memset
#define alloca	__builtin_alloca

/* File */
#include <linux/fcntl.h>
#define stdin	0
#define stdout	1
#define stderr	2
int open(const char *pathname, int flags, ...);
int close(int fd);
int read(int fd, void *buf, unsigned long count);
int write(int fd, const void *buf, unsigned long count);
int lseek(int fd, int offset, int whence);
int ioctl(long fd, unsigned long request, ...);
int fcntl(int fildes, int cmd, ...);

/* Process */
#include <asm-generic/signal.h>
#include <linux/resource.h>
#define exit(code)      asm("jmp _exit_c" :: "a"((unsigned char)code)); __builtin_unreachable();
int execve(const char *filename, char *const argv[], char *const envp[]);
int prlimit64(pid_t pid, int resource, const struct rlimit64 *new_limit, struct rlimit64 *old_limit);
int personality(unsigned long persona);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oact);
int kill(pid_t pid, int sig);
int pause(void);

/* Time */
#include <linux/time.h>
int nanosleep(const struct timespec *req, struct timespec *rem);

/* tty */
#include <asm-generic/termbits.h>

#endif