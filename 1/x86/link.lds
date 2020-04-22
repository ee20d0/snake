SECTIONS
{
	. = 0x10000;
	.text	: { *(.text.head) *(.text.syscall.*) *(.text*) *(.rodata*) *(.data*) }
	.bss	: { *(.bss) *(COMMON) }

	/DISCARD/ :
	{
		*(.note.gnu.property) *(.comment)
	}

	_phys_size	= SIZEOF(.text);
	_virt_size	= SIZEOF(.text) + SIZEOF(.bss);
}
