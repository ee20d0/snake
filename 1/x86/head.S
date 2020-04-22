.section .text.head
.code32
.globl	_start
.globl	_exit_c
.type	_start,@function
.type	_exit_c,@function

#include <elf-common.h>
#include <linux/elf-em.h>

_eh_s:
	// e_ident
		.ascii	ELFMAG		// EI_MAG
		.byte	ELFCLASS32
_start:
		pop	%eax
		push	%esp
		push	%eax
		call	main
_exit_c:	xchg	%eax, %ebx
		jmp	_exit

	.word	ET_EXEC			// e_type
	.word	EM_386			// e_machine
	.long	EV_CURRENT		// e_version
	.long	_start			// e_entry
	.long	SIZEOF_EHDR_32 - 8	// e_phoff (-8 bytes from overlap)
	.long	0			// e_shoff
	.long	DUMMY			// e_flags
	.word	SIZEOF_EHDR_32		// e_ehsize
	.word	SIZEOF_PHDR_32		// e_phentsize
_ph_s:	.word	1			// e_phnum	// p_type
	.word	0			// e_shentsize
	.word	0			// e_shnum	// p_offset
	.word 	0			// e_shstrndx
_eh_e:
	.long	0x00010000		// p_vaddr
	.long	0x00010000		// p_paddr
	.long	_phys_size		// p_filesz
	.long	_virt_size		// p_memsz
	.long	7			// p_flags
	.long	0x1000			// p_align
_ph_e:
_exit:
	xor	%eax, %eax
	inc	%eax		// __NR_exit == $1
	int	$0x80
