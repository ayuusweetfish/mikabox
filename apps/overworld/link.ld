ENTRY(main)

SECTIONS
{
  . = 0x40000000;

  /DISCARD/ : { *(.ARM.* .debug_frame) }

  .text ALIGN(4096) : { *(.text .text.*) }
  .rodata ALIGN(4096) : { *(.rodata .rodata.*) }
  .data ALIGN(4096) : { *(.data .data.*) }
  . = ALIGN(0x8);
  _bss_begin = .;
  .bss : { *(.bss .bss.*) }
  _bss_end = .;

  . = ALIGN(4096);
  _initial_brk = .;
}
