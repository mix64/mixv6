OUTPUT_FORMAT("elf32-i386")
ENTRY(main)

SECTIONS {
	. = 0x40000000;
	.text	: {*(.text)}
	.rodata	: {*(.rodata)}
	. = ALIGN(0x1000);
	.data	: {*(.data)}
	.bss	: {*(.bss)}
}

