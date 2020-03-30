OUTPUT_FORMAT("elf32-i386")
ENTRY(main)

SECTIONS {
	. = 0x100000;
	.text	: {*(.text)}
	.rodata	: {*(.rodata)}
	. = ALIGN(0x1000);
	PROVIDE(rodata_end = .);
	.data	: {*(.data)}
	.bss	: {*(.bss)}
	. = ALIGN(0x1000);
	PROVIDE(kernel_end = .);
}

