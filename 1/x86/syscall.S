.code32
#include <linux/unistd.h>

#define syscall(name)		\
.section .text.syscall. ## name;\
.global name;			\
.type name ##,@function;	\
name ##:			\
	push	$__NR_ ## name;	\
	jmp	_syscall

syscall(open);
syscall(close);
syscall(read);
syscall(write);
syscall(lseek);
syscall(ioctl);
syscall(execve);
syscall(prlimit64);
syscall(personality);
syscall(rt_sigaction);
syscall(rt_sigreturn);
syscall(kill);
syscall(pause);
syscall(nanosleep);
syscall(fcntl);

.section .text.syscall.wrapper
_syscall:
	pop	%eax
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
#ifdef _SIGNAL_NO_ASYNC
	add	$20, %esp
	pop	%ebx
	pop	%ecx
	pop	%edx
	pop	%esi
	pop	%edi
	pop	%ebp
	sub	$44, %esp
#else
	mov	20(%esp), %ebx
	mov	24(%esp), %ecx
	mov	28(%esp), %edx
	mov	32(%esp), %esi
	mov	36(%esp), %edi
	mov	40(%esp), %ebp
#endif
	int	$0x80
	pop	%ebp
	pop	%edi
	pop	%esi
	pop	%ebx
	ret



