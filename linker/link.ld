ENTRY(_start)

SECTIONS
{
	. = 0xffff000000080000;

	.text.boot : { *(.text.boot) }

    . = ALIGN(0x00001000);
	user_begin = .;
	.text.user : {
		*user.o (.text .text.*)
		*memory.o (.text .text.*)
		*string.o (.text .text.*)
	}
	.rodata.user : {
		*user.o (.rodata .rodata.*)
		*memory.o (.rodata .rodata.*)
		*string.o (.rodata .rodata.*)
	}
	.data.user : {
		*user.o (.data .data.*)
		*memory.o (.data . data.*)
		*string.o (.data .data.*)
	}
    . = ALIGN(0x00001000);
	user_end = .;

    .text : { *(.text) }
	__font_start = .;
	.rodata :
	{
		font/font.o
		*(.rodata)
	}
	. = ALIGN(4096);

	.data :
	{
		*(.data)
	}
	. = ALIGN(4096);

    . = ALIGN(0x8);
	__bss_start = .;
	.bss :
	{
		*(.bss)
	}
	.bss.user : { build/user* (.bss) }
	. = ALIGN(4096);
	__bss_end = .;
	__bss_size = __bss_end - __bss_start;
	. = ALIGN(0x00001000);
	pgd = .;
	.data.pgd : { . += (5 * (1 << 12)); }
}
